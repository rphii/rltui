#include "tui-sync.h"

void tui_sync_main_update(Tui_Sync_Main *sync) {
    ASSERT_ARG(sync);
    pthread_mutex_lock(&sync->mtx);
    ++sync->update_do;
    pthread_cond_signal(&sync->cond);
    pthread_mutex_unlock(&sync->mtx);
}

void tui_sync_main_render(Tui_Sync_Main *sync) {
    ASSERT_ARG(sync);
    pthread_mutex_lock(&sync->mtx);
    ++sync->render_do;
    pthread_cond_signal(&sync->cond);
    pthread_mutex_unlock(&sync->mtx);
}


void tui_sync_draw(Tui_Sync_Draw *sync) {
    ASSERT_ARG(sync);
    pthread_mutex_lock(&sync->mtx);
    ++sync->draw_do;
    pthread_cond_signal(&sync->cond);
    pthread_mutex_unlock(&sync->mtx);
}

void tui_sync_redraw(Tui_Sync_Draw *sync) {
    ASSERT_ARG(sync);
    pthread_mutex_lock(&sync->mtx);
    ++sync->draw_do;
    ++sync->draw_redraw;
    pthread_cond_signal(&sync->cond);
    pthread_mutex_unlock(&sync->mtx);
}


void tui_sync_input_idle(Tui_Sync_Input *sync) {
    ASSERT_ARG(sync);
    pthread_mutex_lock(&sync->mtx);
    sync->idle = true;
    pthread_mutex_unlock(&sync->mtx);
}

void tui_sync_input_wake(Tui_Sync_Input *sync) {
    pthread_mutex_lock(&sync->mtx);
    sync->idle = false;
    pthread_cond_signal(&sync->cond);
    pthread_mutex_unlock(&sync->mtx);
}

void tui_sync_input_quit(Tui_Sync_Input *sync) {
    pthread_mutex_lock(&sync->mtx);
    sync->idle = false;
    sync->quit = true;
    pthread_cond_signal(&sync->cond);
    pthread_mutex_unlock(&sync->mtx);
}

