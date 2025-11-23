#include "tui-core.h"
#include "tui-esc-code.h"
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

/* struct s{{{ */

typedef struct Tui_Core {
    Tui_Sync *sync;
    Tui_Buffer buffer;
    Tui_Screen screen;
    Tui_Input_Gen input_gen;
    Tui_Inputs inputs;
    Tui_Core_Callbacks callbacks;
    Pw pw_main;
    Pw pw_draw;
    size_t frames;
    _Atomic bool quit;
    _Atomic bool resized;
    void *user;
} Tui_Core;

/* }}} */

static Tui_Core *g_tui_main;

Tui_Core *tui_global_get(void) {
    return g_tui_main;
}

void tui_global_set(Tui_Core *tui) {
    g_tui_main = tui;
}

void tui_core_signal_winch(int x) {
    Tui_Core *tui = tui_global_get();
    tui->resized = true;
    pthread_cond_signal(&tui->sync->main.cond);
}

void *pw_queue_process_input(Pw *pw, bool *quit, void *void_ctx) {
    Tui_Core *tui = void_ctx;
    for(;;) {
        if(!tui_input_process(&tui->sync->main, &tui->sync->input, &tui->input_gen)) break;
    }
    return 0;
}


void *pw_queue_render(Pw *pw, bool *quit, void *void_ctx) {
    Tui_Core *tui = void_ctx;
    So draw = SO;

    while(!*quit) {

        pthread_mutex_lock(&tui->sync->draw.mtx);
        ++tui->sync->draw.draw_done;
        if(tui->sync->draw.draw_done >= tui->sync->draw.draw_do) {
            tui->sync->draw.draw_do = 0;
            tui->sync->draw.draw_done = 0;
        }
        while(!tui->sync->draw.draw_do && !tui->sync->draw.draw_skip) {
            pthread_cond_wait(&tui->sync->draw.cond, &tui->sync->draw.mtx);
        }
        bool draw_busy = tui->sync->draw.draw_skip;
        bool draw_do = tui->sync->draw.draw_do;
        bool draw_redraw = tui->sync->draw.draw_redraw;
        tui->sync->draw.draw_skip = 0;
        //if(draw_busy && tui->frames > 4) exit(1);
        if(draw_do && !draw_busy && draw_redraw) {
            tui->sync->draw.draw_redraw = 0;
        }
        pthread_mutex_unlock(&tui->sync->draw.mtx);

        if(draw_busy) {
            pthread_mutex_lock(&tui->sync->main.mtx);
            ++tui->sync->main.render_do;
            pthread_cond_signal(&tui->sync->main.cond);
            pthread_mutex_unlock(&tui->sync->main.mtx);
            continue;
        }

        if(!draw_do) continue;

        so_clear(&draw);

        if(draw_redraw) {
            memset(tui->screen.old.cells, 0, sizeof(Tui_Cell) * tui->buffer.dimension.x * tui->buffer.dimension.y);
            so_extend(&draw, so(TUI_ESC_CODE_CURSOR_HIDE));
        }

        tui_screen_fmt(&draw, &tui->screen);

        ssize_t len = draw.len;
        ssize_t written = 0;
        char *begin = so_it0(draw);
        while(written < len) {
            //break;
            errno = 0;
            ssize_t written_chunk = write(STDOUT_FILENO, begin, len - written);
            if(written_chunk > 0) {
                written += written_chunk;
                begin += written_chunk;
            } else {
                if(errno) {
                    printff("\rerrno on write: %u", errno);exit(1);
                } else {
                    continue;
                }
            }
        }
        ++tui->frames;
    }
    return 0;
}

struct Tui_Core *tui_core_new(void) {
    Tui_Core *result;
    NEW(Tui_Core, result);
    return result;
}

int tui_core_init(struct Tui_Core *tui, Tui_Core_Callbacks *callbacks, Tui_Sync *sync, void *user) {
    ASSERT_ARG(tui);
    ASSERT_ARG(sync);

    tui_global_set(tui);
    if(callbacks) {
        tui->callbacks = *callbacks;
    }
    tui->user = user;
    tui->sync = sync;
    tui->sync->main.update_do = true;

    signal(SIGWINCH, tui_core_signal_winch);

    pw_init(&tui->pw_main, 1);
    pw_queue(&tui->pw_main, pw_queue_process_input, tui);
    pw_dispatch(&tui->pw_main);

    pw_init(&tui->pw_draw, 1);
    pw_queue(&tui->pw_draw, pw_queue_render, tui);
    pw_dispatch(&tui->pw_draw);

    return 0;
}


void tui_core_handle_resize(Tui_Core *tui) {
    if(!tui->resized) return;
    
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    Tui_Point dimension = {
        .x = w.ws_col,
        .y = w.ws_row,
    };

    Tui_Point dimension_prev = tui->buffer.dimension;
    if(!tui_point_cmp(dimension_prev, dimension)) {
        tui->resized = false;
        return;
    }

    if(tui->callbacks.render) {
        tui->callbacks.resized(tui, dimension, tui->user);
    }

#if 0
    if(w.ws_xpixel && w.ws_ypixel) {
        tui->aspect_ratio_cell_xy = 2 * ((double)w.ws_xpixel / (double)dimension.x) / ((double)w.ws_ypixel / (double)dimension.y);
    } else {
        tui->aspect_ratio_cell_xy = 1;
    }
    tui->sync_panel.panel_gaki.config.rc = (Tui_Rect){ .dim = dimension };
#endif


    tui_buffer_resize(&tui->buffer, dimension);
    tui_sync_main_render(&tui->sync->main);
}


bool tui_core_loop(Tui_Core *tui) {

    pthread_mutex_lock(&tui->sync->main.mtx);
    bool update_do = tui->sync->main.update_done < tui->sync->main.update_do;
    //printff("\rupdate do:%u",update_do);
    pthread_mutex_unlock(&tui->sync->main.mtx);

    if(update_do) {
        tui_core_handle_resize(tui);

        bool render = false;
        tui_input_get_stack(&tui->sync->input, &tui->inputs);
        bool flush = false;
        if(tui->callbacks.input) {
            while(!tui->quit && array_len(tui->inputs)) {
                Tui_Input input = array_pop(tui->inputs);
                render |= tui->callbacks.input(tui, &input, &flush, tui->user);
                if(flush) continue;
            }
        }

        if(tui->callbacks.update) {
            render |= tui->callbacks.update(tui, tui->user);
        }

        pthread_mutex_lock(&tui->sync->main.mtx);
        ++tui->sync->main.update_done;
        if(render || !tui->frames) ++tui->sync->main.render_do;
        pthread_mutex_unlock(&tui->sync->main.mtx);
    }

    if(tui->quit) return false;

    bool draw_busy = true;
    pthread_mutex_lock(&tui->sync->draw.mtx);
    draw_busy = tui->sync->draw.draw_done < tui->sync->draw.draw_do;
    //printff("\rdraw busy:%u",draw_busy);
    if(draw_busy) {
        ++tui->sync->draw.draw_skip;
    }
    pthread_mutex_unlock(&tui->sync->draw.mtx);

    pthread_mutex_lock(&tui->sync->main.mtx);
    bool render_do = tui->sync->main.render_done < tui->sync->main.render_do;
    //printff("\rrender do:%u",render_do);
    if(draw_busy) {
        tui->sync->main.render_do = tui->sync->main.render_done;
    }
    pthread_mutex_unlock(&tui->sync->main.mtx);


    if(render_do && !draw_busy) {
        tui->buffer.cursor.id = TUI_CURSOR_NONE;

        tui_buffer_clear(&tui->buffer);

        if(tui->callbacks.render) {
            tui->callbacks.render(tui, &tui->buffer, tui->user);
        }

        pthread_mutex_lock(&tui->sync->main.mtx);
        ++tui->sync->main.render_done;
        pthread_mutex_unlock(&tui->sync->main.mtx);

        pthread_mutex_lock(&tui->sync->draw.mtx);
        ++tui->sync->draw.draw_do;
        if(tui_point_cmp(tui->screen.dimension, tui->buffer.dimension)) {
            tui_screen_resize(&tui->screen, tui->buffer.dimension);
        }
        memcpy(tui->screen.now.cells, tui->buffer.cells, sizeof(Tui_Cell) * tui->buffer.dimension.x * tui->buffer.dimension.y);
        tui->screen.now.cursor = tui->buffer.cursor;
        pthread_cond_signal(&tui->sync->draw.cond);
        pthread_mutex_unlock(&tui->sync->draw.mtx);

    }
#if 1
    pthread_mutex_lock(&tui->sync->main.mtx);
    if(tui->sync->main.render_done >= tui->sync->main.render_do) {
        tui->sync->main.render_do = 0;
        tui->sync->main.render_done = 0;
    }
    if(tui->sync->main.update_done >= tui->sync->main.update_do) {
        tui->sync->main.update_do = 0;
        tui->sync->main.update_done = 0;
    }
    while(!tui->sync->main.update_do && !tui->sync->main.render_do) {
        if(tui->resized) {
            tui->sync->main.update_do = true;
            break;
        } else {
            pthread_cond_wait(&tui->sync->main.cond, &tui->sync->main.mtx);
        }
    }
    pthread_mutex_unlock(&tui->sync->main.mtx);
#endif

    return !tui->quit;
}

void tui_core_free(Tui_Core *tui) {
}

int tui_core_quit(struct Tui_Core *tui) {
    tui_sync_input_quit(&tui->sync->input);
}

