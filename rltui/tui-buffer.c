#include "tui-buffer.h"
#include "rlwcwidth.h"

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

void tui_buffer_mono(Tui_Buffer *buf, Tui_Color *fg, Tui_Color *bg, Tui_Fx *fx) {
    Tui_Point pt;
    for(pt.y = 0; pt.y < buf->dimension.y; ++pt.y) {
        for(pt.x = 0; pt.x < buf->dimension.x; ++pt.x) {
            Tui_Cell *cell_curr = tui_buffer_at(buf, pt);
            cell_curr->bg = bg ? *bg : (Tui_Color){0};
            cell_curr->fg = fg ? *fg : (Tui_Color){0};
            cell_curr->fx = fx ? *fx : (Tui_Fx){0};
        }
    }
}

void tui_buffer_draw(Tui_Buffer *buf, Tui_Rect rect, Tui_Color *fg, Tui_Color *bg, Tui_Fx *fx, So so) {
    Tui_Point pt;
    Tui_Rect cnv = { .dim = buf->dimension };
    So line = SO;
    So override = SO;
    So_Uc_Point ucp;
    for(pt.y = rect.anc.y; pt.y < rect.anc.y + rect.dim.y; ++pt.y) {
        if(!so_splice(so, &line, '\n')) break;
        size_t nleft_count = 0;
        size_t nleft_width = 0;
        for(pt.x = rect.anc.x; (pt.x < rect.anc.x + rect.dim.x) || nleft_width; ++pt.x) {

            if(!nleft_count && !nleft_width) {

                So *ref = override.len ? &override : &line;

                if(so_len(*ref)) {
                    so_uc_point(*ref, &ucp);
                    so_shift(ref, ucp.bytes);
                } else {
                    ucp.val = 0;
                }

                if(ucp.val == '\t') {
                    override = so("    ");
                    --pt.x;
                    continue;
                }

                if(!tui_rect_encloses_point(cnv, pt)) continue;
                Tui_Cell *cell = tui_buffer_at(buf, pt);

                if(cell->nleft) {
                    for(size_t i = 1; i <= cell->nleft; ++i) {
                        Tui_Point ptl = { .x = pt.x - i, .y = pt.y };
                        if(ptl.x < 0) break;
                        Tui_Cell *cell_l = tui_buffer_at(buf, ptl);
                        cell_l->ucp.val = 0;
                        cell_l->width = 0;
                        cell_l->nleft = 0;
                    }
                }

                cell->bg = bg ? *bg : (Tui_Color){0};
                cell->fg = fg ? *fg : (Tui_Color){0};
                cell->fx = fx ? *fx : (Tui_Fx){0};

                int w = rlwcwidth(ucp.val);
                int wbound = w;
                if(wbound == 1) --wbound;
                if((wbound + pt.x < rect.anc.x + rect.dim.x) && (wbound + pt.x < buf->dimension.x)) {
                    cell->ucp = ucp;
                    cell->width = w;
                    cell->nleft = 0;
                } else {
                    cell->ucp.val = 0;
                    cell->width = 0;
                    cell->nleft = 0;
                }

                nleft_width = cell->width > 1 ? cell->width - 1 : 0;

            } else {

                if(!tui_rect_encloses_point(cnv, pt)) continue;
                Tui_Cell *cell = tui_buffer_at(buf, pt);
                cell->bg = bg ? *bg : (Tui_Color){0};
                cell->fg = fg ? *fg : (Tui_Color){0};
                cell->fx = fx ? *fx : (Tui_Fx){0};

                cell->nleft = ++nleft_count;
                if(nleft_count >= nleft_width) {
                    nleft_count = 0;
                    nleft_width = 0;
                }
            }

        }
    }
}


