#ifndef TUI_GLOBAL_H

#include <termios.h>
#include "tui-point.h"

struct termios *tui_global_get_termios(void);
void tui_global_set_termios(struct termios *t);
int *tui_global_err_depth(void);
Tui_Point *tui_global_dimensions(void);


#define TUI_GLOBAL_H
#endif

