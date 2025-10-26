#include "tui-rect.h"

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

Tui_Point tui_rect_project_point(Tui_Rect rc, Tui_Point pt) {
    Tui_Point result = pt;
    result.x -= rc.anc.x;
    result.y -= rc.anc.y;
    return result;
}

