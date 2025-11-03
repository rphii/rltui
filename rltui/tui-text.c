#include "tui-text.h"
#include "rlwcwidth.h"

void tui_text_line_clear(Tui_Text_Line *tx) {
    so_clear(&tx->so);
    tx->visual_len = 0;
}

void tui_text_line_fmt(Tui_Text_Line *tx, const char *fmt, ...) {
    So tmp = SO;
    va_list va;
    va_start(va, fmt);
    so_fmt_va(&tmp, fmt, va);
    va_end(va);
    So_Uc_Point ucp;
    for(size_t i = 0; i < so_len(tmp); ++i) {
        int r = so_uc_point(so_i0(tmp, i), &ucp);
        if(r) continue;
        tui_text_line_push(tx, ucp);
        i += (ucp.bytes ? ucp.bytes - 1 : 0);
    }
    so_free(&tmp);
}


void tui_text_line_push(Tui_Text_Line *tx, So_Uc_Point ucp) {
    if(ucp.val >= ' ') {
        so_uc_fmt_point(&tx->so, &ucp);
        tx->visual_len += rlwcwidth(ucp.val);
    }
}

So_Uc_Point tui_text_line_pop(Tui_Text_Line *tx) {
    So_Uc_Point result = {0};
    if(so_len(tx->so)) {
        size_t i = so_len(tx->so);
        while(so_len(tx->so)) {
            unsigned char c = so_at(tx->so, so_len(tx->so) - 1);
            --i;
            if(!(c & 0x80)) break;
        }
        so_uc_point(so_i0(tx->so, i), &result);
        tx->so = so_iE(tx->so, i);
        tx->visual_len -= rlwcwidth(result.val);
    }
    return result;
}


