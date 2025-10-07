#ifndef TUI_RECT_H

#include "tui-point.h"
#include <stdbool.h>

typedef struct Tui_Rect {
    union {
        [[deprecated]] Tui_Point anchor;
        Tui_Point anc;
    };
    union {
        [[deprecated]] Tui_Point dimension;
        Tui_Point dim;
    };
} Tui_Rect;

bool tui_rect_contains_point(Tui_Rect rect, Tui_Point pt);

#define TUI_RECT_H
#endif

