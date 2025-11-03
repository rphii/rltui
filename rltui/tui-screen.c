#include "tui-screen.h"
#include "tui-esc-code.h"
#include "tui-die.h"
#include <rlwcwidth.h>

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

void tui_screen_recalculate_ranges(Tui_Screen *scr) {
    ASSERT_ARG(scr);
    Tui_Point pt;
    Tui_Range y_range = {0};
    bool y_have_i0 = false;
    for(pt.y = 0; pt.y < scr->dimension.y; ++pt.y) {
        Tui_Range x_range = {0};
        bool x_have_iE = false;
        for(pt.x = 0; pt.x < scr->dimension.x; ++pt.x) {
            Tui_Cell *cell_now = tui_buffer_at(&scr->now, pt);
            Tui_Cell *cell_old = tui_buffer_at(&scr->old, pt);
            if(!cell_now->nleft) {
                if(!cell_now->ucp.val) cell_now->ucp.val = ' ';
                //cell_now->width = rlwcwidth(cell_now->ucp.val);
                if(tui_cell_cmp(cell_now, cell_old)) {
                    if(!x_have_iE) {
                        x_range.i0 = pt.x;
                    }
                    int w = cell_now->width > 0 ? cell_now->width : 1;
                    if(pt.x + w <= scr->dimension.x) {
                        x_have_iE = true;
                        x_range.iE = pt.x + w;
                    }
                    if(!y_have_i0) {
                        y_have_i0 = true;
                        y_range.i0 = pt.y;
                    }
                    y_range.iE = pt.y + 1;
                }
            }
            if(!x_have_iE) {
                x_range.i0 = 0;
            }
        }
        //if(x_range.iE > x_range.i0) {
        //    Tui_Cell *cell_now = tui_buffer_at(&scr->now, (Tui_Point){ .x = x_range.i0, .y = pt.y });
        //    Tui_Cell *cell_old = tui_buffer_at(&scr->old, (Tui_Point){ .x = x_range.i0, .y = pt.y });
        //    memcpy(cell_old, cell_now, 
        //}
        scr->x_ranges[pt.y] = x_range;
    }
    scr->y_range = y_range;
}

void tui_screen_fmt(So *out, Tui_Screen *scr) {
    tui_screen_recalculate_ranges(scr);
    //so_fmt(out, TUI_ESC_CODE_CLEAR);
    Tui_Point pt;
    Tui_Range x_range_prev = {0};
    ssize_t y_prev = 0;
    Tui_Cell *cell_prev = 0;
    bool update_cursor = false;
    //scr->y_range.i0 = 0;
    //scr->y_range.iE = scr->dimension.y;
    if(scr->old.cursor.id != scr->now.cursor.id) {
        update_cursor = true;
    }
    if(scr->old.cursor.id || scr->now.cursor.id) {
        so_extend(out, so(TUI_ESC_CODE_CURSOR_HIDE));
    }
    for(pt.y = scr->y_range.i0; pt.y < scr->y_range.iE; ++pt.y) {
        Tui_Range x_range = scr->x_ranges[pt.y];
        //x_range.i0 = 0;
        //x_range.iE = scr->dimension.x;
        size_t x_max_prev = x_range_prev.iE >= scr->dimension.x ? scr->dimension.x - 1 : x_range_prev.iE;
        if(x_range.iE > scr->dimension.x) x_range.iE = scr->dimension.x;

        /* maybe goto somewhere */
#if 0
        if(x_range.i0 || x_range.iE) {
            so_fmt(out, TUI_ESC_CODE_GOTO(x_range.i0, pt.y));
        }
#else
#if 1
        if(!x_range.i0 && x_range.iE && (x_range_prev.i0 || x_range_prev.iE)) {
            if(pt.y - y_prev > 1) {
                so_fmt(out, "\r\e[%uB", pt.y - y_prev);
            } else {
                so_fmt(out, "\r\n");
            }
        } else if(x_range.i0 || x_range.iE) {
            so_fmt(out, TUI_ESC_CODE_GOTO(x_range.i0, pt.y));
        }
#else
        if(!x_range.i0 && x_range.iE && (x_range_prev.i0 || x_range_prev.iE)) {
            if(pt.y - y_prev > 1) {
                so_fmt(out, "\r\e[%uB", pt.y - y_prev);
            } else {
                so_fmt(out, "\r\n");
            }
        } else if(x_range.i0 || x_range.iE) {
            if(!x_range_prev.i0 && !x_range_prev.iE) {
                so_fmt(out, TUI_ESC_CODE_GOTO(x_range.i0, pt.y));
            } else {
                if(x_max_prev > x_range.i0) {
                    so_fmt(out, "\e[%uD\e[%uB", x_max_prev - x_range.i0, pt.y - y_prev);
                } else if(x_max_prev < x_range.i0) {
                    so_fmt(out, "\e[%uC\e[%uB", x_range.i0 - x_max_prev, pt.y - y_prev);
                } else {
                    so_fmt(out, "\n");
                }
            }
        }
#endif
#endif

        for(pt.x = x_range.i0; pt.x < x_range.iE; ++pt.x) {
            /* now actually format the point */
            Tui_Cell *cell_curr = tui_buffer_at(&scr->now, pt);
            if(cell_curr->nleft) continue;
            tui_cell_colordiff_fmt(out, cell_curr, cell_prev);
            update_cursor = true;
#if 1
            //so_extend(out, so("\e[41m"));
            if(so_uc_fmt_point(out, &cell_curr->ucp)) {
                cell_curr->ucp.val = '?';
                cell_curr->ucp.bytes = 1;
                //tui_die("should handle invalid unicode point in buffer.. (should never even happen in the first place)");
            }
            //so_extend(out, so("\e[0m"));
#else
#if 0
            so_fmt(out, "%u", cell_curr->width);
#else
            if(cell_curr->width >= 2) {
                so_fmt(out, "%*s!", cell_curr->width - 1, "X");
            } else {
                so_fmt(out, "%u", cell_curr->width);
            }
#endif
#endif
                //*cell_old = *cell_now; // can actually replace with a memcpy from i0..iE
            cell_prev = cell_curr;
        }

        if(x_range.i0 < x_range.iE) {
            x_range_prev = x_range;
            y_prev = pt.y;
        }
    }
    if(update_cursor && scr->now.cursor.id) {
        so_fmt(out, TUI_ESC_CODE_GOTO(scr->now.cursor.pt.x, scr->now.cursor.pt.y));
        switch(scr->now.cursor.id) {
            case TUI_CURSOR_BAR: {
                so_extend(out, so(TUI_ESC_CODE_CURSOR_BAR));
            } break;
            case TUI_CURSOR_BLOCK: {
                so_extend(out, so(TUI_ESC_CODE_CURSOR_BLOCK));
            } break;
            default: {
                printff("invalid cursor id: %u", scr->now.cursor.id);
                tui_die("see above");
            } break;
        }
        so_extend(out, so(TUI_ESC_CODE_CURSOR_SHOW));
    }
    memcpy(scr->old.cells, scr->now.cells, sizeof(*scr->old.cells) * scr->dimension.x * scr->dimension.y);
    scr->old.cursor = scr->now.cursor;
    tui_buffer_clear(&scr->now);
}

