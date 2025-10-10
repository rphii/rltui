#include "tui-cell.h"

int tui_cell_cmp(Tui_Cell *a, Tui_Cell *b) {
    if(!a && !b) return 0;
    if(!a || !b) return -1;
    if(a->ucp.val != b->ucp.val) return -1;
    if(tui_color_cmp(a->fg, b->fg)) return -1;
    if(tui_color_cmp(a->bg, b->bg)) return -1;
    if(a->nleft != b->nleft) return -1;
    if(tui_fx_cmp(a->fx, b->fx)) return -1;
    //if(a->width != b->width) return -1;
    return 0;
    return memcmp(a, b, sizeof(*a));
}

void tui_cell_colordiff_fmt(So *out, Tui_Cell *a, Tui_Cell *b) {
    ASSERT_ARG(out);
    ASSERT_ARG(a);
    Tui_Color do_fg = {0};
    Tui_Color do_bg = {0};
    Tui_Fx do_fx = {0};
    bool reset_fg = false;
    bool reset_bg = false;
    bool any = false;
    bool delim = false;
    Tui_Fx reset_fx = {0};
    if(b) {
        if(tui_color_cmp(a->fg, b->fg)) {
            do_fg = a->fg;
            if(!do_fg.type) reset_fg = true;
        }
        if(tui_color_cmp(a->bg, b->bg)) {
            do_bg = a->bg;
            if(!do_bg.type) reset_bg = true;
        }
        if(!a->fx.bold && b->fx.bold) reset_fx.bold = true;
        if(!a->fx.it && b->fx.it) reset_fx.it = true;
        if(!a->fx.ul && b->fx.ul) reset_fx.ul = true;
        if(a->fx.bold && !b->fx.bold) do_fx.bold = true;
        if(a->fx.it && !b->fx.it) do_fx.it = true;
        if(a->fx.ul && !b->fx.ul) do_fx.ul = true;
    } else {
        do_fg = a->fg;
        do_bg = a->bg;
        do_fx = a->fx;
        if(!do_fg.type) reset_fg = true;
        if(!do_fg.type) reset_bg = true;
    }
    /* check if we do anything */
    if(reset_fg || reset_bg || do_fg.type || do_bg.type ||
            reset_fx.bold || reset_fx.it || reset_fx.ul) {
        any = true;
        so_extend(out, so("\e["));
    }
    /* check reset */
    if(reset_fg && reset_bg) {
        so_extend(out, so("0"));
        delim = true;
    } else if(reset_fg) {
        so_extend(out, so("39"));
        delim = true;
    } else if(reset_bg) {
        so_extend(out, so("49"));
        delim = true;
    }
    if(reset_fx.bold) {
        if(delim) so_extend(out, so(";"));
        so_extend(out, so("22"));
        delim = true;
    }
    if(reset_fx.it) {
        if(delim) so_extend(out, so(";"));
        so_extend(out, so("23"));
        delim = true;
    }
    if(reset_fx.ul) {
        if(delim) so_extend(out, so(";"));
        so_extend(out, so("24"));
        delim = true;
    }
    /* foreground */
    switch(do_fg.type) {
        case TUI_COLOR_8: {
            if(delim) so_extend(out, so(";"));
            so_fmt(out, "%u", do_fg.col8 + 30);
            delim = true;
        } break;
        case TUI_COLOR_256: {
            if(delim) so_extend(out, so(";"));
            so_fmt(out, "38;5;%u", do_fg.col256);
            delim = true;
        } break;
        case TUI_COLOR_RGB: {
            if(delim) so_extend(out, so(";"));
            so_fmt(out, "38;2;%u;%u;%u", do_fg.r, do_fg.g, do_fg.b);
            delim = true;
        } break;
        default: break;
    }
    /* background */
    switch(do_bg.type) {
        case TUI_COLOR_8: {
            if(delim) so_extend(out, so(";"));
            so_fmt(out, "%u", do_bg.col8 + 40);
            delim = true;
        } break;
        case TUI_COLOR_256: {
            if(delim) so_extend(out, so(";"));
            so_fmt(out, "48;5;%u", do_bg.col256);
            delim = true;
        } break;
        case TUI_COLOR_RGB: {
            if(delim) so_extend(out, so(";"));
            so_fmt(out, "48;2;%u;%u;%u", do_bg.r, do_bg.g, do_bg.b);
            delim = true;
        } break;
        default: break;
    }
    /* fx */
    if(do_fx.bold) {
        if(delim) so_extend(out, so(";"));
        so_extend(out, so("1"));
        delim = true;
    }
    if(do_fx.it) {
        if(delim) so_extend(out, so(";"));
        so_extend(out, so("3"));
        delim = true;
    }
    if(do_fx.ul) {
        if(delim) so_extend(out, so(";"));
        so_extend(out, so("4"));
        delim = true;
    }
    /* done */
    if(any) {
        so_extend(out, so("m"));
        //so_extend(out, so("\b#"));
    }
}

