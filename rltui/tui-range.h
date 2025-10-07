#ifndef TUI_RANGE_H

#include <unistd.h>

typedef struct Tui_Range {
    ssize_t i0;
    ssize_t iE;
} Tui_Range, *Tui_Ranges;

#define TUI_RANGE_H
#endif

