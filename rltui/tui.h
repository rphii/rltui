#ifndef TUI_H

#include <stdbool.h>
#include "tui-input.h"

#define TUI_UNKNOWN_CHARACTER_CSTR   "�"
#define TUI_UNKNOWN_CHARACTER_POINT  0xFFFD

void tui_enter();
void tui_exit();

#define TUI_H
#endif

