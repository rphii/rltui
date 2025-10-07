#ifndef TUI_INPUT_H

#include <stdbool.h>
#include "tui-point.h"
#include <rlso.h>
#include <pthread.h>

#define TUI_INPUT_MAX   128

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
} Tui_Key_List;

typedef enum {
    INPUT_NONE,
    INPUT_TEXT,
    INPUT_KEY,
    INPUT_MOUSE,
} Tui_Input_List;

typedef enum {
    MOUSE_NONE,
    MOUSE_LEFT,
    MOUSE_MIDDLE,
    MOUSE_RIGHT,
    MOUSE_WHEEL,
} Tui_Mouse_List;

#define TUI_INPUT_RAW_MAX   128

typedef struct Tui_Input_Special_Cursor_Position {
    pthread_cond_t cond;
    pthread_mutex_t mtx;
    bool ready;
    Tui_Point point;
} Tui_Input_Special_Cursor_Position;

typedef struct Tui_Input_Special {
    Tui_Input_Special_Cursor_Position cursor_position;
} Tui_Input_Special;

typedef struct Tui_Input_Raw {
    unsigned char c[TUI_INPUT_RAW_MAX];
    unsigned char bytes;
    bool carry_esc;
    //Tui_Mouse_List mouse;
    //int val;
} Tui_Input_Raw;

typedef struct Tui_Mouse {
    Tui_Point pos;
    int scroll;
    bool l;
    bool m;
    bool r;
} Tui_Mouse;

typedef struct Tui_Input {
    Tui_Input_Special special;
    Tui_Input_List id;
    Tui_Key_List key;
    Tui_Mouse mouse;
    So text;
    bool esc;
    bool shift;
    bool ctrl;
    bool alt;
    Tui_Input_Raw _raw;
} Tui_Input;

bool tui_input_process(Tui_Input *ti);
void tui_input_await_cursor_position(Tui_Input_Special_Cursor_Position *pos, Tui_Point *point);

#define TUI_INPUT_H
#endif

