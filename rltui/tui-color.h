#ifndef TUI_COLOR_H

#include <stdint.h>

typedef enum {
    TUI_COLOR_NONE,
    TUI_COLOR_8,
    TUI_COLOR_256,
    TUI_COLOR_RGB,
} Tui_Color_List;

typedef struct Tui_Color {
    union {
        struct {        // full rgb
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        uint8_t col8 : 3;  // 8 colors
        uint8_t col256; // 256 colors
    };
    Tui_Color_List type;
} Tui_Color;

int tui_color_cmp(Tui_Color a, Tui_Color b);

#define TUI_COLOR_H
#endif

