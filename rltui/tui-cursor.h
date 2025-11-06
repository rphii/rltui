#ifndef TUI_CURSOR_H

#include "tui-point.h"
#include <stdbool.h>

typedef enum Tui_Cursor_List {
    TUI_CURSOR_NONE,
    TUI_CURSOR_BLOCK,
    TUI_CURSOR_BAR,
} Tui_Cursor_List;

typedef struct Tui_Cursor {
    Tui_Point pt;
    Tui_Cursor_List id;
} Tui_Cursor;

#define TUI_CURSOR_H
#endif // TUI_CURSOR_H

