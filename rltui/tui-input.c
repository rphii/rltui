#include "tui-input.h"
#include "tui-write.h"
#include "tui-die.h"
#include "tui-sync.h"
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
    if(!kbhit()) {
        //usleep(1e1);
        //return 0;
    }
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
    decode->id = INPUT_NONE;
    if(input->bytes == 0) {
        bool changed = false;
#if 0
        if(decode->esc || decode->key.down ||
           decode->alt.down || decode->ctrl.down ||
           decode->shift.down) {
            changed = true;
        }
        /* reset key states (keep mouse as is) */
        decode->esc.down = false;
        decode->key.down = false;
        decode->alt.down = false;
        decode->ctrl.down = false;
        decode->shift.down = false;
#endif
        return changed;
    }
    if(!iscntrl(input->c[0])) {
        decode->id = INPUT_TEXT;
        if(so_uc_point(so_ll((char *)input->c, input->bytes), &decode->text)) {
            return false;
        }
        //decode->key.down = true;
        return true;
    }
    if(input->bytes == 1) {
        switch(*input->c) {
            case 0x1b: decode->code = KEY_CODE_ESC; break;
            case 0x0d: decode->code = KEY_CODE_ENTER; break;
            case 0x7f: decode->code = KEY_CODE_BACKSPACE; break;
            default: return INPUT_NONE;
        }
        //decode->key.down = true;
        decode->id = INPUT_CODE;
    } else if(input->bytes > 3 && input->c[input->bytes - 1] == 'R') {
        So in = so_ll((char *)input->c + 2, input->bytes - 3);
        So right, left = so_split_ch(in, ';', &right);
        Tui_Input_Special_Cursor_Position *pos = &decode->special.cursor_position;
        int error = 0;
        error |= so_as_ssize(left, &pos->point.y, 10);
        error |= so_as_ssize(right, &pos->point.x, 10);
        if(!error) {
            pthread_mutex_lock(&pos->mtx);
            pos->ready = true;
            pthread_cond_signal(&pos->cond);
            pthread_mutex_unlock(&pos->mtx);
        }
    } else if(input->bytes > 2 && input->c[0] == '\x1b' && input->c[1] == '[') {
        int in_offs = 2;
        So in = so_ll(input->c + in_offs, input->bytes - 2);
        size_t len = so_len(in);
        if(len == 1) {
            switch(so_at(in, 0)) {
                case 'A': decode->code = KEY_CODE_UP; break;
                case 'B': decode->code = KEY_CODE_DOWN; break;
                case 'C': decode->code = KEY_CODE_RIGHT; break;
                case 'D': decode->code = KEY_CODE_LEFT; break;
                default: return INPUT_NONE;
            }
            //decode->key.down = true;
            decode->id = INPUT_CODE;
        } else if(so_at(in, 0) == '<' && len >= 2) {
            decode->mouse = mouse_prev;
            decode->id = INPUT_MOUSE;
            size_t iE = so_find_any(in, so("mM"));
            in = so_iE(in, iE < in.len ? iE + 1 : iE);
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
                decode->mouse.l.down = false;
                decode->mouse.m.down = false;
                decode->mouse.r.down = false;
            }
            /* post process */
            decode->mouse.scroll = 0;
            switch(mouse_id) {
                case MOUSE_LEFT: decode->mouse.l.down = mouse_val; break;
                case MOUSE_MIDDLE: decode->mouse.m.down = mouse_val; break;
                case MOUSE_RIGHT: decode->mouse.r.down = mouse_val; break;
                case MOUSE_WHEEL: decode->mouse.scroll = mouse_val; break;
                default: break;
            }
            if(iE + 1 + in_offs < input->bytes) {
                input->next = iE + 1 + in_offs;
            }
            //printff("WHEEL:%i",decode->mouse.scroll);
        }
    }
    return decode->id != INPUT_NONE;
}


bool tui_input_process_raw(Tui_Input_Raw *raw, Tui_Input *input) {
    ASSERT_ARG(raw);
    ASSERT_ARG(input);
    bool result = tui_input_decode(raw, input);
    return result;
}

Tui_Input_State tui_input_state(Tui_Input_State now, Tui_Input_State old) {
    Tui_Input_State result = {0};
    if(now.down > old.down) {
        result.press = true;
        result.down = true;
    } else if(now.down && old.down) {
        result.repeat = true;
        result.down = true;
    } else if(now.down < old.down) {
        result.release = true;
    }
    return result;
}

bool tui_input_process(Tui_Sync_Main *sync_m, Tui_Sync_Input *sync, Tui_Input_Gen *gen) {
    ASSERT_ARG(sync);
    ASSERT_ARG(gen);
    bool done = false;
    bool loop = false;
    gen->old = gen->now;
    tui_input_get(&gen->raw);
    while(!done) {
        Tui_Input process = gen->now;
        if(loop) {
            gen->old = process;
        }
        if(!tui_input_decode(&gen->raw, &process)) break;
        Tui_Input input = process;
        gen->now = process;
        //input.alt = tui_input_state(input.alt, gen->old.alt);
        //input.esc = tui_input_state(input.esc, gen->old.esc);
        //input.key = tui_input_state(input.key, gen->old.key);
        //input.ctrl = tui_input_state(input.ctrl, gen->old.ctrl);
        //input.shift = tui_input_state(input.shift, gen->old.shift);
        input.mouse.l = tui_input_state(input.mouse.l, gen->old.mouse.l);
        input.mouse.r = tui_input_state(input.mouse.r, gen->old.mouse.r);
        input.mouse.m = tui_input_state(input.mouse.m, gen->old.mouse.m);

        pthread_mutex_lock(&sync->mtx);
        array_push(sync->inputs, input);
        pthread_mutex_unlock(&sync->mtx);
        tui_sync_main_update(sync_m);
        loop = true;

        if(gen->raw.next && gen->raw.next < gen->raw.bytes) {
            memmove(gen->raw.c, gen->raw.c + gen->raw.next, gen->raw.bytes - gen->raw.next);
            gen->raw.bytes = gen->raw.bytes - gen->raw.next;
            gen->raw.next = 0;
        } else {
            done = true;
        }

    }

    pthread_mutex_lock(&sync->mtx);
    while(!sync->quit && sync->idle) {
        pthread_cond_wait(&sync->cond, &sync->mtx);
    }
    bool quit = sync->quit;
    pthread_mutex_unlock(&sync->mtx);
    return !quit;
}

void tui_input_get_stack(Tui_Sync_Input *sync, Tui_Inputs *inputs) {
    ASSERT_ARG(sync);
    ASSERT_ARG(inputs);
    pthread_mutex_lock(&sync->mtx);
    size_t len = array_len(sync->inputs);
    for(size_t i = 0; i < len; ++i) {
        array_push(*inputs, array_pop(sync->inputs));
    }
    pthread_mutex_unlock(&sync->mtx);
}

void tui_input_await_cursor_position(Tui_Input_Special_Cursor_Position *pos, Tui_Point *point) {
    pthread_mutex_lock(&pos->mtx);
    pos->ready = false;
    tui_write_cstr("\e[6n");
    while(!pos->ready) {
        pthread_cond_wait(&pos->cond, &pos->mtx);
    }
    pthread_mutex_unlock(&pos->mtx);
    *point = pos->point;
}

