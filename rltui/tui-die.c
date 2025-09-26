#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <rlso.h>
#include <unistd.h>

#include "tui-raw.h"
#include "tui-die.h"
#include "tui-write.h"
#include "tui-global.h"

void tui_die(char *fmt, ...) {
    tui_raw_disable();
    int *depth = tui_global_err_depth();
    if((*depth)++) return;
    So msg = {0};
    va_list va;
    va_start(va, fmt);
    so_fmt_va(&msg, fmt, va);
    va_end(va);
    perror(fmt);
    fprintf(stderr, "to get your previous display back, enter: tput rmcup\n");
    fprintf(stderr, "if that doesn't help, then enter: reset\n");
    exit(1);
}

