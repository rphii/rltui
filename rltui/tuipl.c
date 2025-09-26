#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <rlso.h>
#include <rlc.h>
#include <math.h>
#include <sys/ioctl.h>


#ifndef ESC_CODE_H

#define ESC_CODE_CLEAR          "\e[2J"
#define ESC_CODE_GOTO(x,y)      "\e[%u;%uH", (y)+1, (x)+1
#define ESC_CODE_HOME           "\e[H"
#define ESC_CODE_CURSOR_HIDE    "\e[?25l"
#define ESC_CODE_CURSOR_SHOW    "\e[?25h"
#define ESC_CODE_CURSOR_DEFAULT "\e[0 q"
#define ESC_CODE_CURSOR_BAR     "\e[6 q"
#define ESC_CODE_CURSOR_BLOCK   "\e[2 q"
#define ESC_CODE_MOUSE_ON       "\e[?1003h\e[?1015h\e[?1006h"
#define ESC_CODE_MOUSE_OFF      "\e[?1000l"

#define ESC_CODE_LEN(x)     sizeof(x)-1

#define ESC_CODE_H
#endif

struct termios termios_entry;

typedef unsigned char byte;

#define ri_write_cstr(s) \
    write(STDOUT_FILENO, s, sizeof(s)-1)

#define ri_write_nstr(s, n) \
    write(STDOUT_FILENO, s, n)

#define INPUT_MAX   128

typedef struct Input {
    byte c[INPUT_MAX];
    byte bytes;
    bool carry_esc;
} Input;

void die(const char *s) {
    ri_write_cstr("\e[?1000l");
    perror(s);
    fprintf(stderr, "to get your previous display back, enter: tput rmcup\n");
    fprintf(stderr, "if that doesn't help, then enter: reset\n");
    exit(1);
}

int input_get_byte(byte *c) {
    int nread;
    if ((nread = read(STDIN_FILENO, c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return nread;
}

int input_get(Input *input) {
    byte c;
    input->bytes = 0;
    int have_c = false;
    if(input->carry_esc) {
        c = 0x1b;
        have_c = true;
    } else {
        have_c = input_get_byte(&c);
    }
    input->carry_esc = false;
    if(have_c) {
        if(c >= 0xC0) {
            byte bytes = 0;
            if(c < 0x80) bytes = 1;
            else if((c & 0xE0) == 0xC0) bytes = 2;
            else if((c & 0xF0) == 0xE0) bytes = 3;
            else if((c & 0xF8) == 0xF0) bytes = 4;
            input->bytes = bytes;
            if(bytes > 0) input->c[0] = c;
            if(bytes > 1) input_get_byte(&input->c[1]);
            if(bytes > 2) input_get_byte(&input->c[2]);
            if(bytes > 3) input_get_byte(&input->c[3]);
        } else if (c == 0x1b) {
            byte bytes = 0;
            input->c[bytes] = c;
            while(bytes + 1 < INPUT_MAX && input_get_byte(&input->c[++bytes])) {
                if(input->c[bytes] == 0x1b) {
                    input->carry_esc = true;
                    break;
                }
            }
            input->bytes = bytes;
        } else {
            input->bytes = 1;
            input->c[0] = c;
        }
    }
    return input->bytes;
}

typedef enum {
    KEY_NONE,
    KEY_ESC,
    KEY_ENTER,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_BACKSPACE,
    KEY_MOUSE,
} Key_List;

typedef enum {
    INPUT_NONE,
    INPUT_TEXT,
    INPUT_KEY,
    INPUT_MOUSE,
} Input_List;

typedef enum {
    MOUSE_NONE,
    MOUSE_LEFT,
    MOUSE_MIDDLE,
    MOUSE_RIGHT,
    MOUSE_WHEEL,
} Mouse_List;

typedef struct Point {
    ssize_t x;
    ssize_t y;
} Point;

typedef struct Input_Decode {
    Input_List id;
    Key_List key;
    Mouse_List mouse;
    Point pos;
    int val;
    bool esc;
    bool shift;
    bool ctrl;
    bool alt;
} Input_Decode;

void input_decode(Input *input, Input_Decode *decode) {
    *decode = (Input_Decode){0};
    if(input->bytes == 0) return;
    if(!iscntrl(input->c[0])) return;
    if(input->bytes == 1) {
        switch(*input->c) {
            case 0x1b: decode->key = KEY_ESC; break;
            case 0x0d: decode->key = KEY_ENTER; break;
            case 0x7f: decode->key = KEY_BACKSPACE; break;
            default: return;
        }
        decode->id = INPUT_KEY;
    } else if(input->bytes > 2 && input->c[0] == '\x1b' && input->c[1] == '[') {
        So in = so_ll(input->c + 2, input->bytes - 2);
        size_t len = so_len(in);
        if(len == 1) {
            switch(so_at(in, 0)) {
                case 'A': decode->key = KEY_UP; break;
                case 'B': decode->key = KEY_DOWN; break;
                case 'C': decode->key = KEY_RIGHT; break;
                case 'D': decode->key = KEY_LEFT; break;
                default: return;
            }
            decode->id = INPUT_KEY;
        } else if(so_at(in, 0) == '<' && len >= 2) {
            decode->id = INPUT_MOUSE;
            byte end = so_atE(in);
            So sonums = so_sub(in, 1, len - 1), sonum = SO;
            size_t i = 0;
            for(sonum = SO, i = 0; so_splice(sonums, &sonum, ';'); ++i) {
                if(so_is_zero(sonum)) { --i; continue; }
                unsigned int num;
                if(so_as_uint(sonum, &num, 10)) continue;
                if(i == 0) {
                    if(num == 0) decode->mouse = MOUSE_LEFT;
                    if(num == 1) decode->mouse = MOUSE_MIDDLE;
                    if(num == 2) decode->mouse = MOUSE_RIGHT;
                    if(num <= 2) decode->val = (end == 'M');
                    if(num & 0x40) {
                        decode->mouse = MOUSE_WHEEL;
                        decode->val = num & 0x01 ? 1 : -1;
                    }
                    decode->alt = (num & 0x08);
                    decode->ctrl = (num & 0x10);
                }
                if(i == 1) decode->pos.x = num - 1;
                if(i == 2) decode->pos.y = num - 1;
            }
        }
    }
}

void disable_raw_mode() {
    ri_write_cstr(ESC_CODE_CURSOR_SHOW);
    ri_write_cstr(ESC_CODE_MOUSE_OFF);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_entry) == -1) {
        die("tcsetattr");
    }
    system("tput rmcup");
}

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &termios_entry) == -1) die("tcgetattr");
    atexit(disable_raw_mode);
    system("tput smcup");
    struct termios raw = termios_entry;
    cfmakeraw(&raw);
    //raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    //raw.c_oflag &= ~(OPOST);
    //raw.c_cflag |= (CS8);
    //raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
    ri_write_cstr(ESC_CODE_MOUSE_ON);
}

int cursor_pos_get(int *x, int *y) {
    char buf[32];
    unsigned int i = 0;
    if(write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = 0;
    if(buf[0] != '\x1b' || buf[1] != '[') return -1;
    char *endptr;
    int try_y = strtoul(&buf[2], &endptr, 10);
    int offs = endptr - buf;
    if(buf[offs++] != ';') return -1;
    int try_x = strtoul(&buf[offs], &endptr, 10);
    offs = endptr - buf;
    if(buf[offs] != 0) return -1;
    *y = try_y;
    *x = try_x;
    return -1;
}

byte input_get_width_via_write(Input input) {
    if(input.bytes == 0) return 0;
    if(input.bytes == 1 && !iscntrl(input.c[0])) {
        return 1;
    }
    if(input.bytes >= 1 && iscntrl(input.c[0])) return 0;
    int x0, xE, y0, yE;
    cursor_pos_get(&x0, &y0);
    write(STDOUT_FILENO, input.c, input.bytes);
    cursor_pos_get(&xE, &yE);
    return xE - x0;
}

bool point_is_in_bounds(ssize_t x, ssize_t x0, ssize_t x1) {
    bool result = false;
    if(x0 < x1) {
        result = x >= x0 && x < x1;
    } else {
        result = x >= x1 && x < x0;
    }
    return result;
}

bool point_is_in_rect(Point point, Point a, Point b) {
    bool is_x = point_is_in_bounds(point.x, a.x, b.x);
    bool is_y = point_is_in_bounds(point.y, a.y, b.y);
    bool result = is_x && is_y;
    return result;
}

typedef struct Mouse {
    Point pos;
    int scroll;
    bool l;
    bool m;
    bool r;
} Mouse;

#define DIMENSION_X     40
#define DIMENSION_Y     20

#define COORD_Y_FINAL   1
#define COORD_Y_RD      3
#define COORD_Y_GN      4
#define COORD_Y_BL      5

int main0(void) {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    Point dimension = { .x = w.ws_col, .y = w.ws_row };
    if(dimension.x < DIMENSION_X || dimension.y < DIMENSION_Y) return -1;

    Color color = {0};

    enable_raw_mode();

    byte *intensity = 0;
    Input input = {0};
    Input_Decode input_dc = {0};
    Mouse mouse_prev = {0};
    Mouse mouse = {0};
    So out = {0};
    bool quit = false;
    while(!quit) {
        if(!input_get(&input)) continue;
        mouse_prev = mouse;
        input_decode(&input, &input_dc);
        switch(input_dc.id) {
            case INPUT_KEY: {
                switch(input_dc.key) {
                    case KEY_ESC: quit = true; break;
                    default: break;
                }
            } break;
            case INPUT_MOUSE: {
                mouse.pos = input_dc.pos;
                mouse.scroll = 0;
                switch(input_dc.mouse) {
                    case MOUSE_LEFT: {
                        mouse.l = input_dc.val;
                    } break;
                    case MOUSE_MIDDLE: mouse.m = input_dc.val; break;
                    case MOUSE_RIGHT: mouse.r = input_dc.val; break;
                    case MOUSE_WHEEL: mouse.scroll = input_dc.val; break;
                    default: break;
                }
            } break;
            default: break;
        }
        int n = 30;
        int offs = 6;
        /* process input */
        byte *mod = 0;
        if(mouse.l && !mouse_prev.l) {
            if(mouse.pos.x >= offs && mouse.pos.x < n + offs) {
                if(mouse.pos.y == COORD_Y_RD) mod = &color.r;
                if(mouse.pos.y == COORD_Y_GN) mod = &color.g;
                if(mouse.pos.y == COORD_Y_BL) mod = &color.b;
                intensity = mod;
            }
        }
        if(!mouse.l) intensity = 0;
        if(mouse.l) {
            if(intensity) {
                if(mouse.pos.x >= offs && mouse.pos.x < n + offs) {
                    *intensity = 255.0f / (float)(n - 1) * (float)(mouse.pos.x - offs);
                } else if(mouse.pos.x < offs) {
                    *intensity = 0;
                } else if(mouse.pos.x >= n + offs) {
                    *intensity = 255;
                }
            }
        }
        /* render */
        so_clear(&out);
        so_extend(&out, so(ESC_CODE_CURSOR_HIDE));
        so_extend(&out, so(ESC_CODE_CLEAR));
        so_extend(&out, so(ESC_CODE_HOME));
        //so_fmt(&out, "x,y    = %u,%u\r\n", mouse.pos.x, mouse.pos.y);
        //so_fmt(&out, "L,M,R  = %u,%u,%u\r\n", mouse.l, mouse.m, mouse.r);
        //so_fmt(&out, "scroll = %d\r\n", mouse.scroll);
        //so_fmt(&out, "ctrl   = %d\r\n", input_dc.ctrl);
        //so_fmt(&out, "alt    = %d\r\n", input_dc.alt);
        /* draw color bar : r */
        so_fmt(&out, ESC_CODE_GOTO(0, COORD_Y_RD));
        so_fmt_fx(&out, (So_Fx){ .bg.r = color.r }, 0, "R=%u", color.r);
        so_fmt(&out, ESC_CODE_GOTO(offs, COORD_Y_RD));
        for(size_t i = 0; i < n; ++i) {
            byte v = (byte)((float)i * 255.0f / (float)(n));
            char *x = roundf((float)color.r / 255.0f * (float)(n - 1)) == i ? "|" : "·";
            so_fmt_fx(&out, (So_Fx){ .bg.r = v }, 0, "%s", x);
        }
        /* draw color bar : g */
        so_fmt(&out, ESC_CODE_GOTO(0, COORD_Y_GN));
        so_fmt_fx(&out, (So_Fx){ .bg.g = color.g }, 0, "G=%u", color.g);
        so_fmt(&out, ESC_CODE_GOTO(offs, COORD_Y_GN));
        for(size_t i = 0; i < n; ++i) {
            byte v = (byte)((float)i * 255.0f / (float)(n));
            char *x = roundf((float)color.g / 255.0f * (float)(n - 1)) == i ? "|" : "·";
            so_fmt_fx(&out, (So_Fx){ .bg.g = v }, 0, "%s", x);
        }
        /* draw color bar : b */
        so_fmt(&out, ESC_CODE_GOTO(0, COORD_Y_BL));
        so_fmt_fx(&out, (So_Fx){ .bg.b = color.b }, 0, "B=%u", color.b);
        so_fmt(&out, ESC_CODE_GOTO(offs, COORD_Y_BL));
        for(size_t i = 0; i < n; ++i) {
            byte v = (byte)((float)i * 255.0f / (float)(n));
            char *x = roundf((float)color.b / 255.0f * (float)(n - 1)) == i ? "|" : "·";
            so_fmt_fx(&out, (So_Fx){ .bg.b = v }, 0, "%s", x);
        }
        /* draw final color */
        so_fmt(&out, ESC_CODE_GOTO(0, COORD_Y_FINAL));
        so_extend(&out, so("FINAL"));
        so_fmt(&out, ESC_CODE_GOTO(offs, COORD_Y_FINAL));
        so_fmt_fx(&out, (So_Fx){ .bg = color }, 0, "#%06x%*s", ((size_t)color.r<<16) + ((size_t)color.g<<8) + color.b, n-7, "");
        /* draw cursor */
        if(point_is_in_rect(mouse.pos, (Point){0}, dimension) && !mouse.l) {
            so_fmt(&out, ESC_CODE_GOTO(mouse.pos.x, mouse.pos.y));
            so_extend(&out, so(ESC_CODE_CURSOR_SHOW));
        }
        write(STDOUT_FILENO, so_it0(out), out.len);
    }
    so_free(&out);
    return 0;
}


