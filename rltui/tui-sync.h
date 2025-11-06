#ifndef TUI_SYNC_H

#include <pthread.h>
#include "tui-input.h"

typedef struct Tui_Sync_Main {
    pthread_cond_t cond;
    pthread_mutex_t mtx;
    unsigned int update_do;
    unsigned int update_done;
    unsigned int render_do;
    unsigned int render_done;
} Tui_Sync_Main;

typedef struct Tui_Sync_Draw {
    pthread_cond_t cond;
    pthread_mutex_t mtx;
    unsigned int draw_do;
    unsigned int draw_skip;
    unsigned int draw_done;
    unsigned int draw_redraw;
} Tui_Sync_Draw;

typedef struct Tui_Sync_Input {
    pthread_cond_t cond;
    pthread_mutex_t mtx;
    bool idle;
    bool quit;
    Tui_Inputs inputs;
    Tui_Input_Gen gen;
} Tui_Sync_Input;

void tui_sync_main_update(Tui_Sync_Main *sync);
void tui_sync_main_render(Tui_Sync_Main *sync);
void tui_sync_main_both(Tui_Sync_Main *sync);

void tui_sync_draw(Tui_Sync_Draw *sync);
void tui_sync_redraw(Tui_Sync_Draw *sync);

void tui_sync_input_idle(Tui_Sync_Input *sync);
void tui_sync_input_wake(Tui_Sync_Input *sync);
void tui_sync_input_quit(Tui_Sync_Input *sync);

#define TUI_SYNC_H
#endif // TUI_SYNC_H


