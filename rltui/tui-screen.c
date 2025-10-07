#include "tui-screen.h"
#include "tui-esc-code.h"
//#include <rlwcwidth.h>

void tui_screen_resize(Tui_Screen *scr, Tui_Point dimension) {
    ASSERT_ARG(scr);
    tui_buffer_resize(&scr->now, dimension);
    tui_buffer_resize(&scr->old, dimension);
    array_resize(scr->x_ranges, dimension.y);
    tui_buffer_clear(&scr->old);
    scr->dimension = dimension;
}

void tui_screen_free(Tui_Screen *scr) {
    if(!scr) return;
    tui_buffer_free(&scr->now);
    tui_buffer_free(&scr->old);
    array_free(scr->x_ranges);
    memset(scr, 0, sizeof(*scr));
}

void tui_screen_recalculate_ranges(Tui_Screen *buf) {
    ASSERT_ARG(buf);
    Tui_Point pt;
    Tui_Range y_range = {0};
    bool y_have_i0 = false;
    for(pt.y = 0; pt.y < buf->dimension.y; ++pt.y) {
        Tui_Range x_range = {0};
        bool x_have_i0 = false;
        for(pt.x = 0; pt.x < buf->dimension.x; ++pt.x) {
            Tui_Cell *cell_now = tui_buffer_at(&buf->now, pt);
            Tui_Cell *cell_old = tui_buffer_at(&buf->old, pt);
            if(!cell_now->ucp.val) cell_now->ucp.val = ' ';
            //cell_now->width = rlwcwidth(cell_now->ucp.val);
            if(tui_cell_cmp(cell_now, cell_old)) {
                if(!x_have_i0) {
                    x_have_i0 = true;
                    x_range.i0 = pt.x;
                }
                x_range.iE = pt.x + 1;
                if(!y_have_i0) {
                    y_have_i0 = true;
                    y_range.i0 = pt.y;
                }
                y_range.iE = pt.y + 1;
                *cell_old = *cell_now;
            }
        }
        buf->x_ranges[pt.y] = x_range;
        //printff("\rxrange %u..%u",x_range.i0,x_range.iE);
    }
    buf->y_range = y_range;
    tui_buffer_clear(&buf->now);
}

void tui_screen_fmt(So *out, Tui_Screen *buf) {
    tui_screen_recalculate_ranges(buf);
    Tui_Point pt;
    Tui_Range x_range_prev = {0};
    for(pt.y = buf->y_range.i0; pt.y < buf->y_range.iE; ++pt.y) {
        Tui_Range x_range = buf->x_ranges[pt.y];

        /* maybe goto somewhere */
        if(!x_range_prev.i0 && !x_range_prev.iE) {
            so_fmt(out, TUI_ESC_CODE_GOTO(x_range.i0, pt.y));
            //printff("\rGOTO %u,%u",x_range.i0,pt.y);usleep(1e4);
        } else if(x_range.i0) {
            if(x_range.i0 < x_range_prev.iE) {
                ssize_t n = x_range_prev.iE - x_range.i0;
                so_fmt(out, "\e[%zuD", n);
            } else if(x_range.i0 > x_range_prev.iE) {
                ssize_t n = x_range.i0 - x_range_prev.iE;
                so_fmt(out, "\e[%zuC", n);
            }
        } else {
            so_push(out, '\r');
        }

        for(pt.x = x_range.i0; pt.x < x_range.iE; ++pt.x) {
            /* now actually format the point */
            Tui_Cell *cell = tui_buffer_at(&buf->old, pt);
            so_uc_fmt_point(out, &cell->ucp);
        }
        x_range_prev = x_range;

        if(pt.y + 1 < buf->y_range.iE) {
            so_push(out, '\n');
        }
    }
}

