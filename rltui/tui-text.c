#include "tui-text.h"
#include "rlwcwidth.h"

void tui_text_line_push(Tui_Text_Line *tx, So_Uc_Point ucp) {
    so_uc_fmt_point(&tx->so, &ucp);
    if(ucp.val >= ' ') {
        tx->visual_len += rlwcwidth(ucp.val);
    }
}

So_Uc_Point tui_text_line_pop(Tui_Text_Line *tx) {
    So_Uc_Point result = {0};
    if(so_len(tx->so)) {
        size_t i = so_len(tx->so);
        while(so_len(tx->so)) {
            unsigned char c = so_at(tx->so, so_len(tx->so));
            --i;
            if(!(c & 0x80)) break;
        }
        so_uc_point(so_i0(tx->so, i), &result);
        tx->visual_len -= rlwcwidth(result.val);
    }
    return result;
}


