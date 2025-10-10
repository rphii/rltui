#ifndef TUI_SCREEN_H

#include "tui-buffer.h"
#include "tui-range.h"
#include "tui-rect.h"

typedef struct Tui_Screen {
    Tui_Buffer now;
    Tui_Buffer old;
    Tui_Ranges x_ranges;
    Tui_Range  y_range;
    Tui_Point  dimension;
} Tui_Screen;

void tui_screen_resize(Tui_Screen *scr, Tui_Point dimension);
void tui_screen_free(Tui_Screen *scr);
void tui_screen_fmt(So *out, Tui_Screen *scr);

#define TUI_SCREEN_H
#endif


