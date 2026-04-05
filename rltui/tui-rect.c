#include "tui-rect.h"

int tui_rect_cmp(Tui_Rect a, Tui_Rect b) {
    if(tui_point_cmp(a.anc, b.anc)) return -1;
    if(tui_point_cmp(a.dim, b.dim)) return -1;
    return 0;
}

bool tui_rect_contains_point(Tui_Rect rect, Tui_Point pt) {
    if(!rect.dim.y || !rect.dim.x) {
        return false;
    }
    if(pt.y >= rect.anc.y && pt.y <= rect.anc.y + rect.dim.y) {
        if(pt.x >= rect.anc.x && pt.x <= rect.anc.x + rect.dim.x) {
            return true;
        }
    }
    return false;
}

bool tui_rect_encloses_point(Tui_Rect rect, Tui_Point pt) {
    if(!rect.dim.y || !rect.dim.x) {
        return false;
    }
    if(pt.y >= rect.anc.y && pt.y < rect.anc.y + rect.dim.y) {
        if(pt.x >= rect.anc.x && pt.x < rect.anc.x + rect.dim.x) {
            return true;
        }
    }
    return false;
}

bool tui_rect_contains_rect(Tui_Rect rect, Tui_Rect b) {
    bool result = true;
    result &= tui_rect_contains_point(rect, (Tui_Point){ .x = b.anc.x, .y = b.anc.y });
    result &= tui_rect_contains_point(rect, (Tui_Point){ .x = b.anc.x + b.dim.x, .y = b.anc.y + b.dim.y });
    return result;
}

bool tui_rect_encloses_rect(Tui_Rect rect, Tui_Rect b) {
    bool result = true;
    result &= tui_rect_encloses_point(rect, (Tui_Point){ .x = b.anc.x, .y = b.anc.y });
    result &= tui_rect_encloses_point(rect, (Tui_Point){ .x = b.anc.x + b.dim.x, .y = b.anc.y + b.dim.y });
    return result;
}


Tui_Point tui_rect_project_point(Tui_Rect rc, Tui_Point pt) {
    Tui_Point result = pt;
    result.x -= rc.anc.x;
    result.y -= rc.anc.y;
    return result;
}

