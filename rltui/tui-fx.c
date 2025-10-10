#include "tui-fx.h"

int tui_fx_cmp(Tui_Fx a, Tui_Fx b) {
    if(a.it != b.it) return -1;
    if(a.ul != b.ul) return -1;
    if(a.bold != b.bold) return -1;
    if(a.strike != b.strike) return -1;
    if(a.wiggle != b.wiggle) return -1;
    return 0;
}

