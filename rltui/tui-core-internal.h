#ifndef RLTUI_CORE_INTERNAL_H

#include "tui-core.h"
#include "tui-esc-code.h"
#include "tui-image.h"

/* structs {{{ */

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
    So buf_draw;
    bool is_graphics_support_ok;
    bool is_graphics_support_queried;
    Tui_Images images;
    pthread_mutex_t mtx_write;
    pthread_mutex_t mtx_tmp;
    So tmp;
} Tui_Core;

/* }}} */

#define RLTUI_CORE_INTERNAL_H
#endif /* RLTUI_CORE_INTERNAL_H */

