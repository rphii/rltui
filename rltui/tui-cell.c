#include "tui-cell.h"

int tui_cell_cmp(Tui_Cell *a, Tui_Cell *b) {
    return memcmp(a, b, sizeof(*a));
}

