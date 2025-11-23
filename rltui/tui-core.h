#ifndef RLTUI_CORE_H

#include "tui-screen.h"
#include "tui-buffer.h"
#include "tui-sync.h"
#include "tui-point.h"
#include <rlpw.h>

struct Tui_Core;

typedef bool (*Tui_Core_Input_Callback)(Tui_Input *input, bool *flush, void *user);
typedef bool (*Tui_Core_Update_Callback)(void *user);
typedef void (*Tui_Core_Render_Callback)(Tui_Buffer *buffer, void *user);
typedef void (*Tui_Core_Resized_Callback)(Tui_Point size, Tui_Point pixels, void *user);

typedef struct Tui_Core_Callbacks {
    Tui_Core_Input_Callback input;
    Tui_Core_Update_Callback update;
    Tui_Core_Render_Callback render;
    Tui_Core_Resized_Callback resized;
} Tui_Core_Callbacks;

struct Tui_Core *tui_core_new(void);
int tui_core_init(struct Tui_Core *tui, Tui_Core_Callbacks *callbacks, Tui_Sync *sync, void *user);
int tui_core_quit(struct Tui_Core *tui);
void tui_core_free(struct Tui_Core *tui);
bool tui_core_loop(struct Tui_Core *tui);


#define RLTUI_CORE_H
#endif // RLTUI_CORE_H

