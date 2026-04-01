#include <stdbool.h>
#include "tui-core-internal.h"
#include "tui-image.h"

bool tui_image_is_supported(Tui_Core *core) {
    if(!core->is_graphics_support_queried) {
        core->is_graphics_support_ok = tui_input_await_image_support(&core->input_gen.special.kitty_graphics);
        core->is_graphics_support_queried = true;
    }
    return core->is_graphics_support_ok;
}

