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

void tui_buffer_draw_cache(Tui_Buffer *buf, Tui_Buffer_Cache *cache, So so) {
    ASSERT_ARG(cache);
    Tui_Rect rect = cache->rect;
    Tui_Point offs = cache->offs;
    Tui_Color *fg = cache->fg;
    Tui_Color *bg = cache->bg;
    Tui_Fx *fx = cache->fx;
    bool fill = cache->fill;
    Tui_Point pt0 = cache->pt;

    Tui_Point pt;
    Tui_Rect cnv = { .dim = buf->dimension };
    So line = SO;
    So override = SO;
    So_Uc_Point ucp;
    bool first = true;
    //printff("\rHELLO");
    for(pt.y = rect.anc.y + pt0.y + offs.y; pt.y < rect.anc.y + rect.dim.y; ++pt.y) {

        if(!first) {
            cache->pt.x = 0;
            ++cache->pt.y;
        }
        first = false;

        if(!so_splice(so, &line, '\n') && !fill) break;
        if(pt.y < rect.anc.y + pt0.y) continue;

        size_t nleft_count = 0;
        size_t nleft_width = 0;
        so_clear(&override);
        for(pt.x = rect.anc.x + pt0.x + offs.x; (pt.x < rect.anc.x + rect.dim.x) || nleft_width; ++pt.x) {

            bool out_bounds = (pt.x < rect.anc.x + pt0.x);

            if(!nleft_count && !nleft_width) {

                So *ref = override.len ? &override : &line;

                if(so_len(*ref)) {
                    if(so_uc_point(*ref, &ucp)) {
                        override = so("?");
                        if(so_len(*ref)) so_shift(ref, 1);
                    } else {
                        so_shift(ref, ucp.bytes);
                    }
                } else {
                    ucp.val = 0;
                    if(!fill) break;
                }

                if(ucp.val == '\t') {
                    override = so("    ");
                    --pt.x;
                    continue;
                }

                if(ucp.val < ' ' && ucp.val > 0) {
                    continue;
                }

                if(!tui_rect_encloses_point(cnv, pt)) continue;
                if(out_bounds) continue;
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
                cache->pt.x += w;
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

                if(!tui_rect_encloses_point(cnv, pt) || out_bounds) {
                    nleft_width = 0;
                    nleft_count = 0;
                    continue;
                }
                Tui_Cell *cell = tui_buffer_at(buf, pt);
                cell->bg = bg ? *bg : (Tui_Color){0};
                cell->fg = fg ? *fg : (Tui_Color){0};
                cell->fx = fx ? *fx : (Tui_Fx){0};
                //cache->pt.x += pt.x;
                ++cache->pt.x;

                cell->nleft = ++nleft_count;
                if(nleft_count >= nleft_width) {
                    nleft_count = 0;
                    nleft_width = 0;
                }
            }

        }

    }
    if(fill) {
        cache->pt.x = 0;
        ++cache->pt.y;
    }
quit:; // semicolon to remove warning
}

void tui_buffer_draw(Tui_Buffer *buf, Tui_Rect rect, Tui_Color *fg, Tui_Color *bg, Tui_Fx *fx, So so) {
    Tui_Buffer_Cache tbc = {
        .fx = fx,
        .bg = bg,
        .fg = fg,
        .fill = true,
        .rect = rect,
    };
    tui_buffer_draw_cache(buf, &tbc, so);
#if 0
    Tui_Point pt;
    Tui_Rect cnv = { .dim = buf->dimension };
    So line = SO;
    So override = SO;
    So_Uc_Point ucp;
    for(pt.y = rect.anc.y; pt.y < rect.anc.y + rect.dim.y; ++pt.y) {
        if(!so_splice(so, &line, '\n')) break;
        size_t nleft_count = 0;
        size_t nleft_width = 0;
        so_clear(&override);
        for(pt.x = rect.anc.x; (pt.x < rect.anc.x + rect.dim.x) || nleft_width; ++pt.x) {

            if(!nleft_count && !nleft_width) {

                So *ref = override.len ? &override : &line;

                if(so_len(*ref)) {
                    if(so_uc_point(*ref, &ucp)) {
                        override = so("?");
                        if(so_len(*ref)) so_shift(ref, 1);
                    } else {
                        so_shift(ref, ucp.bytes);
                    }
                } else {
                    ucp.val = 0;
                }

                if(ucp.val == '\t') {
                    override = so("    ");
                    --pt.x;
                    continue;
                }

                if(ucp.val < ' ' && ucp.val > 0) {
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
#endif
}

void tui_buffer_draw_subbuf(Tui_Buffer *buf, Tui_Rect rect, Tui_Buffer *sub) {
    for(size_t y = rect.anc.y, ys = 0; y < rect.anc.y + rect.dim.y; ++y, ++ys) {
        if(y >= buf->dimension.y) continue;
        if(ys >= sub->dimension.y) continue;
        for(size_t x = rect.anc.x, xs = 0; x < rect.anc.x + rect.dim.x; ++x, ++xs) {
            if(x >= buf->dimension.x) continue;
            if(xs >= sub->dimension.x) continue;
            Tui_Point pt_dst = { .x = x, .y = y };
            Tui_Point pt_src = { .x = xs, .y = ys };
            Tui_Cell *dst = tui_buffer_at(buf, pt_dst);
            Tui_Cell *src = tui_buffer_at(sub, pt_src);
            *dst = *src;
        }
    }
}

