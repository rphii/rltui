#include "tui-point.h"

int tui_point_cmp(Tui_Point a, Tui_Point b) {
    int result = 0;
    result += (a.x != b.x);
    result += (a.y != b.y);
    return result;
}

