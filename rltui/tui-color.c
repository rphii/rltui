#include "tui-color.h"

int tui_color_cmp(Tui_Color a, Tui_Color b) {
    if(a.type != b.type) return 1;
    switch(a.type) {
        case TUI_COLOR_8: {
            return a.col8 != b.col8;
        } break;
        case TUI_COLOR_256: {
            return a.col256 != b.col256;
        } break;
        case TUI_COLOR_RGB: {
            return ((a.r != b.r) || (a.g != b.g) || (a.b != b.b));
        } break;
        default: break;
    }
    return 0;
}

