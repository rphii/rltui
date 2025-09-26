#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <rlso.h>
#include <rlc.h>
#include <sys/ioctl.h>

#include "tui-raw.h"

void tui_enter(void) {
    system("tput smcup");
    tui_raw_enable();
}

void tui_exit(void) {
    tui_raw_disable();
    system("tput rmcup");
}


