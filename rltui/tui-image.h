#ifndef RLTUI_IMAGE_H

#include "tui-rect.h"
#include <stdint.h>
#include <rlso.h>

struct Tui_Core;
struct Tui_Buffer;

bool tui_image_is_supported(struct Tui_Core *core);

typedef struct Tui_Image {
    uint8_t *data;
    Tui_Rect src;
    Tui_Rect dst;
    Tui_Point dimensions;
    int32_t z;
    int channels;
    So kitty_gfx;
    uint32_t id;
} Tui_Image, **Tui_Images;

Tui_Image *tui_image_new(struct Tui_Core *core, uint32_t id, uint8_t *data, Tui_Point dimensions, int channels);
void tui_image_free(struct Tui_Core *core, Tui_Image *image);

int tui_image_update(struct Tui_Core *core, Tui_Image *image, So *errmsg);

void tui_image_config(Tui_Image *image, Tui_Rect src, Tui_Rect dst, int32_t z);
int tui_image_render(struct Tui_Core *core, Tui_Image *image, uint32_t place_id, So *errmsg);

int tui_image_clear_id_image(struct Tui_Core *core, uint32_t image_id);
int tui_image_clear_id_place(struct Tui_Core *core, uint32_t image_id, uint32_t place_id);
int tui_image_clear_all(struct Tui_Core *core);

#define RLTUI_IMAGE_H
#endif /* RLTUI_IMAGE_H */

