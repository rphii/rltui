#include "../rltui.h"

struct Tui_Core *core;

bool input(Tui_Input *input, bool *flush, void *user) {
    printf("\r\nimage support query...\r\n");
    bool supported = tui_image_is_supported(core);
    printf("\r\nimage support: %u\r", supported);
    if(input->id == INPUT_TEXT) {
        if(input->text.val == 'q') {
            tui_core_quit(core);
            return false;
        }
    }
    return true;
}

bool update(void *user) {
    return false;
}

void render(Tui_Buffer *buffer, void *user) {
}

void resized(Tui_Point size, Tui_Point pixels, void *user) {
}


int main(void) {

    core = tui_core_new();
    struct Tui_Core_Callbacks callbacks = {
        .input = input,
        .update = update,
        .render = render,
        .resized = resized,
    };
    struct Tui_Sync sync = {0};

    tui_enter();
    tui_core_init(core, &callbacks, &sync, 0);
    while(tui_core_loop(core)) {}
    tui_core_free(core);
    //tui_exit();

    return 0;
}

