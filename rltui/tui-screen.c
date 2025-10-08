#include "tui-screen.h"
#include "tui-esc-code.h"
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
                    if(pt.x + w < scr->dimension.x) {
                        x_have_iE = true;
                        x_range.iE = pt.x + w;
                    }
                    if(!y_have_i0) {
                        y_have_i0 = true;
                        y_range.i0 = pt.y;
                    }
                    y_range.iE = pt.y + 1;
                }
                *cell_old = *cell_now; // can actually replace with a memcpy from i0..iE
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
    //memcpy(scr->old.cells, scr->now.cells, sizeof(*scr->old.cells) * scr->dimension.x * scr->dimension.y);
    scr->y_range = y_range;
    tui_buffer_clear(&scr->now);
}

void tui_screen_fmt(So *out, Tui_Screen *scr) {
    tui_screen_recalculate_ranges(scr);
    Tui_Point pt;
    Tui_Range x_range_prev = {0};
    for(pt.y = scr->y_range.i0; pt.y < scr->y_range.iE; ++pt.y) {
        Tui_Range x_range = scr->x_ranges[pt.y];
        size_t x_max = x_range.iE >= scr->dimension.x ? scr->dimension.x - 1 : x_range.iE;
        size_t x_max_prev = x_range_prev.iE >= scr->dimension.x ? scr->dimension.x - 1 : x_range_prev.iE;
        if(x_range.iE > scr->dimension.x) x_range.iE = scr->dimension.x;

        /* maybe goto somewhere */
#if 0
            so_fmt(out, TUI_ESC_CODE_GOTO(x_range.i0, pt.y));
#else
        if(!x_range_prev.i0 && !x_range_prev.iE) {
            so_fmt(out, TUI_ESC_CODE_GOTO(x_range.i0, pt.y));
        } else if(x_range.i0) {
            if(x_range.i0 < x_max_prev) {
                ssize_t n = x_max_prev - x_range.i0;
                so_fmt(out, "\e[%zuD", n);
            } else if(x_range.i0 > x_max_prev) {
                ssize_t n = x_range.i0 - x_max_prev;
                so_fmt(out, "\e[%zuC", n);
            }
        } else {
            so_push(out, '\r');
        }
#endif

        //int lastw = 0;
        for(pt.x = x_range.i0; pt.x < x_range.iE; ++pt.x) {
            /* now actually format the point */
            Tui_Cell *cell = tui_buffer_at(&scr->old, pt);
            if(cell->nleft) continue;
            so_uc_fmt_point(out, &cell->ucp);
        }
        x_range_prev = x_range;
        //x_range_prev.iE += lastw;

        if(pt.y + 1 < scr->y_range.iE) {
            so_push(out, '\n');
        }
    }
}

void tui_screen_draw(Tui_Screen *scr, Tui_Rect rect, So so) {
    Tui_Point pt;
    Tui_Rect cnv = { .dim = scr->dimension };
    So line = SO;
    So_Uc_Point ucp;
    for(pt.y = rect.anc.y; pt.y < rect.anc.y + rect.dim.y; ++pt.y) {
        if(!so_splice(so, &line, '\n')) break;
        size_t nleft_count = 0;
        size_t nleft_width = 0;
        for(pt.x = rect.anc.x; pt.x < rect.anc.x + rect.dim.x; ++pt.x) {

            if(!nleft_count && !nleft_width) {

                if(!so_len(line)) break;
                so_uc_point(line, &ucp);
                so_shift(&line, ucp.bytes);

                if(!tui_rect_encloses_point(cnv, pt)) continue;
                Tui_Cell *cell = tui_buffer_at(&scr->now, pt);

                if(cell->nleft) {
                    for(size_t i = 1; i < cell->nleft; ++i) {
                        Tui_Point ptl = { .x = pt.x - i, .y = pt.y };
                        Tui_Cell *cell_l = tui_buffer_at(&scr->now, ptl);
                        cell_l->ucp.val = 0;
                    }
                }

                int w = rlwcwidth(ucp.val);
                if(w + pt.x < rect.dim.x) {
                    cell->width = w;
                    cell->ucp = ucp;
                    cell->nleft = 0;
                } else {
                    cell->ucp.val = ' ';
                }

                nleft_width = cell->width > 1 ? cell->width - 1 : 0;

            } else {

                if(!tui_rect_encloses_point(cnv, pt)) continue;
                Tui_Cell *cell = tui_buffer_at(&scr->now, pt);

                cell->nleft = ++nleft_count;
                if(nleft_count >= nleft_width) {
                    nleft_count = 0;
                    nleft_width = 0;
                }
            }

        }
    }
}

