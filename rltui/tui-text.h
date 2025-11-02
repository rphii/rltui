#ifndef TUI_TEXT_H

#include <rlso.h>

typedef struct Tui_Text_Line {
    So so;
    size_t visual_len;
} Tui_Text_Line;

void tui_text_line_push(Tui_Text_Line *tx, So_Uc_Point ucp);
So_Uc_Point tui_text_line_pop(Tui_Text_Line *tx);

#define TUI_TEXT_H
#endif // TUI_TEXT_H

