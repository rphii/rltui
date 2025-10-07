#ifndef TUI_WRITE_H

#define tui_write_cstr(s) \
    write(STDOUT_FILENO, s, sizeof(s)-1)

#define tui_write_nstr(s, n) \
    write(STDOUT_FILENO, s, (n))

#define tui_write_so(so) \
    write(STDOUT_FILENO, so_it0((so)), so_len((so)))

#define TUI_WRITE_H
#endif

