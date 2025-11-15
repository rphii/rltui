#ifndef RLTUI_MAIN_H

#include "tui-screen.h"
#include "tui-buffer.h"
#include "tui-sync.h"
#include "tui-point.h"
#include <rlpw.h>

struct Tui_Main;

typedef bool (*Tui_Main_Input_Callback)(struct Tui_Main *tui, Tui_Input *input, void *user);
typedef bool (*Tui_Main_Update_Callback)(struct Tui_Main *tui, void *user);
typedef void (*Tui_Main_Render_Callback)(struct Tui_Main *tui, Tui_Buffer *buffer, void *user);
typedef void (*Tui_Main_Resized_Callback)(struct Tui_Main *tui, Tui_Point size, void *user);

struct Tui_Main *tui_main_new(void);
int tui_main_init(struct Tui_Main *tui);
void tui_main_free(struct Tui_Main *tui);
bool tui_main_loop(struct Tui_Main *tui);


#define RLTUI_MAIN_H
#endif // RLTUI_MAIN_H

