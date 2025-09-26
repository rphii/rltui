#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <rlso.h>
#include <rlc.h>
#include <sys/ioctl.h>

#include "tui-global.h"
#include "tui-esc-code.h"
#include "tui-write.h"
#include "tui-die.h"
#include "tui-raw.h"

void tui_raw_enable(void) {
    struct termios orig;
    if(tcgetattr(STDIN_FILENO, &orig) == -1) tui_die("tcgetattr");
    tui_global_set_termios(&orig);
    atexit(tui_raw_disable);
    struct termios raw = orig;
    cfmakeraw(&raw);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) tui_die("tcsetattr");
    tui_write_cstr(TUI_ESC_CODE_MOUSE_ON);
    tui_write_cstr(TUI_ESC_CODE_CURSOR_HIDE);
}

void tui_raw_disable(void) {
    struct termios *orig = tui_global_get_termios();
    if(!orig) return;
    tui_write_cstr(TUI_ESC_CODE_CURSOR_SHOW);
    tui_write_cstr(TUI_ESC_CODE_MOUSE_OFF);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, orig) == -1) {
        tui_die("tcsetattr");
    }
}


