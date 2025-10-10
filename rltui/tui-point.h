#ifndef TUI_POINT_H

#include <unistd.h>

typedef struct Tui_Point {
    ssize_t x;
    ssize_t y;
} Tui_Point;

int tui_point_cmp(Tui_Point a, Tui_Point b);

#define TUI_POINT_H
#endif

