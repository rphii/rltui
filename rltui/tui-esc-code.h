#ifndef TUI_ESC_CODES_H

#define TUI_ESC_CODE_CLEAR          "\e[2J"
#define TUI_ESC_CODE_CLEAR_LINE     "\e[2K"
#define TUI_ESC_CODE_CLEAR_TO_END   "\e[K"
#define TUI_ESC_CODE_GOLINE(y)      "\e[%uH", (y)+1
#define TUI_ESC_CODE_GOTO(x,y)      "\e[%u;%uH", (y)+1, (x)+1
#define TUI_ESC_CODE_HOME           "\e[H"
#define TUI_ESC_CODE_CURSOR_HIDE    "\e[?25l"
#define TUI_ESC_CODE_CURSOR_SHOW    "\e[?25h"
#define TUI_ESC_CODE_CURSOR_DEFAULT "\e[0 q"
#define TUI_ESC_CODE_CURSOR_BAR     "\e[6 q"
#define TUI_ESC_CODE_CURSOR_BLOCK   "\e[2 q"
#define TUI_ESC_CODE_MOUSE_ON       "\e[?1000;1003;1006h"
#define TUI_ESC_CODE_MOUSE_OFF      "\e[?1000;1003;1006l"

#define TUI_ESC_CODE_LEN(x)     sizeof(x)-1

#define TUI_ESC_CODES_H
#endif

