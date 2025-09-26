#ifndef TUI_RECT_H

#include "tui-point.h"

typedef struct Tui_Rect {
    Tui_Point anchor;
    Tui_Point dimension;
} Tui_Rect;

#define TUI_RECT_H
#endif

