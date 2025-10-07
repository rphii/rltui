#ifndef TUI_CELL_H

#include "tui-color.h"
#include "tui-fx.h"
#include <rlso.h>

typedef struct Tui_Cell {
    So_Uc_Point ucp;
    Tui_Color fg;
    Tui_Color bg;
    Tui_Fx fx;
    uint8_t width;  // store character width
    uint8_t nleft;  // allows backtracking multi-cell width characters. if nleft==1 then 2 wide. if nleft==2 then 3 wide. so if any value contained here, it invalidates all other fields
} Tui_Cell, *Tui_Cells;

int tui_cell_cmp(Tui_Cell *a, Tui_Cell *b);

#define TUI_CELL_H
#endif

