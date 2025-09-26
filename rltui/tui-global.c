#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <rlso.h>
#include <rlc.h>
#include <math.h>
#include <sys/ioctl.h>

#include "tui-die.h"
#include "tui-global.h"

static struct termios static_global_termios;
static bool static_global_termios_have_value;

static int static_global_err_depth;

static Tui_Point static_global_dimensions;

struct termios *tui_global_get_termios(void) {
    if(!static_global_termios_have_value) return 0;
    return &static_global_termios;
}

void tui_global_set_termios(struct termios *t) {
    if(!t) return;
    if(static_global_termios_have_value) {
        tui_die("cannot set termios state twice");
    }
    static_global_termios = *t;
    static_global_termios_have_value = true;
}

int *tui_global_err_depth(void) {
    return &static_global_err_depth;
}

Tui_Point *tui_global_dimensions(void) {
    return &static_global_dimensions;
}

