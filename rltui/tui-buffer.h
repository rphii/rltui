#ifndef TUI_BUFFER_H

#include "tui-cell.h"
#include "tui-rect.h"
#include "tui-point.h"

typedef struct Tui_Buffer {
    Tui_Cells cells;
    Tui_Point dimension;
} Tui_Buffer;

void tui_buffer_resize(Tui_Buffer *buf, Tui_Point dimension);
void tui_buffer_clear(Tui_Buffer *buf);
void tui_buffer_free(Tui_Buffer *buf);
Tui_Cell *tui_buffer_at(Tui_Buffer *buf, Tui_Point pt);
void tui_buffer_mono(Tui_Buffer *buf, Tui_Color *fg, Tui_Color *bg, Tui_Fx *fx);
void tui_buffer_draw(Tui_Buffer *buf, Tui_Rect rect, Tui_Color *fg, Tui_Color *bg, Tui_Fx *fx, So so);

#define TUI_BUFFER_H
#endif

