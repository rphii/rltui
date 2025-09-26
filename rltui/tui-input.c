#include "tui-input.h"
#include "tui-die.h"
#include <errno.h>
#include <ctype.h>
#include <rlso.h>

int kbhit(void) {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

int tui_input_get_byte(unsigned char *c) {
    int nread;
    if ((nread = read(STDIN_FILENO, c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) tui_die("read");
    }
    return nread;
}

int tui_input_get(Tui_Input_Raw *input) {
    //if(!kbhit()) return 0;
    unsigned char c;
    input->bytes = 0;
    int have_c = false;
    if(input->carry_esc) {
        c = 0x1b;
        have_c = true;
    } else {
        have_c = tui_input_get_byte(&c);
    }
    input->carry_esc = false;
    if(have_c) {
        if(c >= 0xC0) {
            unsigned char bytes = 0;
            if(c < 0x80) bytes = 1;
            else if((c & 0xE0) == 0xC0) bytes = 2;
            else if((c & 0xF0) == 0xE0) bytes = 3;
            else if((c & 0xF8) == 0xF0) bytes = 4;
            input->bytes = bytes;
            if(bytes > 0) input->c[0] = c;
            if(bytes > 1) tui_input_get_byte(&input->c[1]);
            if(bytes > 2) tui_input_get_byte(&input->c[2]);
            if(bytes > 3) tui_input_get_byte(&input->c[3]);
        } else if (c == 0x1b) {
            unsigned char bytes = 0;
            input->c[bytes] = c;
            while(bytes + 1 < TUI_INPUT_RAW_MAX && tui_input_get_byte(&input->c[++bytes])) {
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

bool tui_input_decode(Tui_Input_Raw *input, Tui_Input *decode) {
    Tui_Mouse mouse_prev = decode->mouse;
    *decode = (Tui_Input){ ._raw = *input, };
    if(input->bytes == 0) return false;
    if(!iscntrl(input->c[0])) {
        decode->id = INPUT_TEXT;
        decode->text = so_ll(input->c, input->bytes);
        return true;
    }
    if(input->bytes == 1) {
        decode->id = INPUT_KEY;
        switch(*input->c) {
            case 0x1b: decode->key = KEY_ESC; break;
            case 0x0d: decode->key = KEY_ENTER; break;
            case 0x7f: decode->key = KEY_BACKSPACE; break;
            default: decode->id = INPUT_NONE; break;
        }
    } else if(input->bytes > 2 && input->c[0] == '\x1b' && input->c[1] == '[') {
        So in = so_ll(input->c + 2, input->bytes - 2);
        size_t len = so_len(in);
        if(len == 1) {
            switch(so_at(in, 0)) {
                case 'A': decode->key = KEY_UP; break;
                case 'B': decode->key = KEY_DOWN; break;
                case 'C': decode->key = KEY_RIGHT; break;
                case 'D': decode->key = KEY_LEFT; break;
                default: return false;
            }
            decode->id = INPUT_KEY;
        } else if(so_at(in, 0) == '<' && len >= 2) {
            //printff("MOUSE%u",rand());
            decode->mouse = mouse_prev;
            decode->id = INPUT_MOUSE;
            unsigned char end = so_atE(in);
            So sonums = so_sub(in, 1, len - 1), sonum = SO;
            size_t i = 0;
            Tui_Mouse_List mouse_id = MOUSE_NONE;
            int mouse_val;
            for(sonum = SO, i = 0; so_splice(sonums, &sonum, ';'); ++i) {
                if(so_is_zero(sonum)) { --i; continue; }
                unsigned int num;
                if(so_as_uint(sonum, &num, 10)) continue;
                if(i == 0) {
                    if(num == 0) mouse_id = MOUSE_LEFT;
                    if(num == 1) mouse_id = MOUSE_MIDDLE;
                    if(num == 2) mouse_id = MOUSE_RIGHT;
                    if(num <= 2) mouse_val = (end == 'M');
                    if(num & 0x40) {
                        mouse_id = MOUSE_WHEEL;
                        mouse_val = num & 0x01 ? 1 : -1;
                    }
                    decode->alt = (num & 0x08);
                    decode->ctrl = (num & 0x10);
                }
                if(i == 1) decode->mouse.pos.x = num - 1;
                if(i == 2) decode->mouse.pos.y = num - 1;
            }
            if(end == 'm') {
                mouse_val = (end == 'M');
                decode->mouse.l = false;
                decode->mouse.m = false;
                decode->mouse.r = false;
            }
            /* post process */
            decode->mouse.scroll = 0;
            switch(mouse_id) {
                case MOUSE_LEFT: decode->mouse.l = mouse_val; break;
                case MOUSE_MIDDLE: decode->mouse.m = mouse_val; break;
                case MOUSE_RIGHT: decode->mouse.r = mouse_val; break;
                case MOUSE_WHEEL: decode->mouse.scroll = mouse_val; break;
                default: break;
            }
            //printff("WHEEL:%i",decode->mouse.scroll);
        }
    }
    return decode->id != INPUT_NONE;
}


bool tui_input_process(Tui_Input *ti) {
    Tui_Input_Raw _raw = {0};
    if(!tui_input_get(&ti->_raw)) return false;
    bool result = tui_input_decode(&ti->_raw, ti);
    return result;
}

