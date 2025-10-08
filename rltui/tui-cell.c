#include "tui-cell.h"

int tui_cell_cmp(Tui_Cell *a, Tui_Cell *b) {
    //int result = a->width - b->width;
    //if(result) return result;
    //result = a->nleft - b->nleft;
    //if(result) return result;
    //result = a->ucp.val - b->ucp.val;
    //if(result) return result;
    //return 0;
    return memcmp(a, b, sizeof(*a));
}

