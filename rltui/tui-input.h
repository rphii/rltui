#ifndef TUI_INPUT_H

#include <stdbool.h>
#include "tui-point.h"
#include <rlso.h>
#include <pthread.h>

#define TUI_INPUT_MAX   128

typedef struct Tui_Sync_Input Tui_Sync_Input;
typedef struct Tui_Sync_Main Tui_Sync_Main;

typedef enum {
    KEY_CODE_NONE,
    KEY_CODE_ESC,
    KEY_CODE_ENTER,
    KEY_CODE_UP,
    KEY_CODE_DOWN,
    KEY_CODE_LEFT,
    KEY_CODE_RIGHT,
    KEY_CODE_BACKSPACE,
    KEY_CODE_MOUSE,
} Tui_Key_Code_List;

typedef enum {
    INPUT_NONE,
    INPUT_TEXT,
    INPUT_CODE,
    INPUT_MOUSE,
} Tui_Input_List;

typedef struct Tui_Input_State {
    bool down:1;
    bool press:1;
    bool repeat:1;
    bool release:1;
} Tui_Input_State;

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
    unsigned int next;
    bool carry_esc;
} Tui_Input_Raw;

typedef struct Tui_Mouse {
    Tui_Point pos;
    int scroll;
    Tui_Input_State l;
    Tui_Input_State m;
    Tui_Input_State r;
} Tui_Mouse;

typedef struct Tui_Input {
    Tui_Input_Special special;
    Tui_Input_List id;
    Tui_Key_Code_List code;
    Tui_Mouse mouse;
    So_Uc_Point text;
    bool esc;
    bool shift;
    bool ctrl;
    bool alt;
} Tui_Input, *Tui_Inputs;

typedef struct Tui_Input_Gen {
    Tui_Input now;
    Tui_Input old;
    Tui_Input_Raw raw;
} Tui_Input_Gen;

bool tui_input_process_raw(Tui_Input_Raw *raw, Tui_Input *input);
bool tui_input_process(Tui_Sync_Main *sync_m, Tui_Sync_Input *sync, Tui_Input_Gen *gen);
void tui_input_get_stack(Tui_Sync_Input *sync, Tui_Inputs *inputs);
void tui_input_await_cursor_position(Tui_Input_Special_Cursor_Position *pos, Tui_Point *point);

#define TUI_INPUT_H
#endif

