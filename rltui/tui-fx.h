#ifndef TUI_FX_H

#include <stdbool.h>

typedef struct Tui_Fx {
    bool bold;
    bool it;
    bool ul;
    bool strike;
    bool wiggle;
} Tui_Fx;

int tui_fx_cmp(Tui_Fx a, Tui_Fx b);

#define TUI_FX_H
#endif

