#include "tui-buffer.h"

void tui_buffer_resize(Tui_Buffer *buf, Tui_Point dimension) {
    ASSERT_ARG(buf);
    array_resize(buf->cells, dimension.x * dimension.y);
    buf->dimension = dimension;
}

void tui_buffer_clear(Tui_Buffer *buf) {
    if(!buf) return;
    if(!buf->cells) return;
    memset(buf->cells, 0, sizeof(Tui_Cell) * buf->dimension.x * buf->dimension.y);
}

void tui_buffer_free(Tui_Buffer *buf) {
    if(!buf) return;
    array_free(buf->cells);
    memset(buf, 0, sizeof(*buf));
}

Tui_Cell *tui_buffer_at(Tui_Buffer *buf, Tui_Point pt) {
    ASSERT_ARG(buf);
    return array_it(buf->cells, buf->dimension.x * pt.y + pt.x);
}

