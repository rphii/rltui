#include <stdbool.h>
#include "tui-image.h"
#include "tui-core-internal.h"
#include "tui-write.h"

#define KITTY_GFX_BEGIN     "\e_G"
#define KITTY_GFX_END       "\e\\"

bool tui_image_is_supported(struct Tui_Core *core) {
    if(!core->is_graphics_support_queried) {
        core->is_graphics_support_ok = tui_input_await_image_support(&core->input_gen.special);
        core->is_graphics_support_queried = true;
    }
    return core->is_graphics_support_ok;
}

Tui_Image *tui_image_new(struct Tui_Core *core, uint32_t id, uint8_t *data, Tui_Point dimensions, int channels) {
    Tui_Image *img;
    NEW(Tui_Image, img);
    array_push(core->images, img);
    img->channels = channels;
    img->data = data;
    img->dimensions = dimensions;
    img->id = id;
    return img;
}

void tui_image_free(struct Tui_Core *core, Tui_Image *image) {

}

void tui_image_kitty_gfx_send(Tui_Image *image) {

    So data = so_ll(image->data, image->dimensions.x * image->dimensions.y * image->channels);

    So base64 = SO;
    so_base64_fmt_encode(&base64, data);

    so_clear(&image->kitty_gfx);
    so_fmt(&image->kitty_gfx, KITTY_GFX_BEGIN "i=%u,f=%u,s=%u,v=%u", image->id, image->channels * 8, image->dimensions.x, image->dimensions.y);

    //so_fmt(&image->kitty_gfx, "a=T,x=%u,y=%u,f=%u,s=%u,v=%u", 0, 0, ch*8, xnew, ynew);
    //so_fmt(&image->kitty_gfx, "q=1,i=%u,f=%u,s=%u,v=%u", task->i, ch*8, x, y);
    unsigned int chunk = 4000;
    if(so_len(base64) > chunk) {
        for(size_t i = 0; i < so_len(base64); i += chunk) {
            bool more =  i + chunk < so_len(base64);
            if(!i) {
                so_fmt(&image->kitty_gfx, ",m=1;");
            } else {
                so_fmt(&image->kitty_gfx, KITTY_GFX_END KITTY_GFX_BEGIN "m=%u;", more);
            }
            if(more) {
                So sub = so_sub(base64, i, i + chunk + 1);
                so_extend(&image->kitty_gfx, sub);
            } else {
                So sub = so_i0(base64, i);
                so_extend(&image->kitty_gfx, sub);
            }
        }
    } else {
        so_fmt(&image->kitty_gfx, ";");
        so_extend(&image->kitty_gfx, base64);
    }
    so_fmt(&image->kitty_gfx, KITTY_GFX_END);

    so_free(&base64);

}

void tui_image_kitty_gfx_place(Tui_Image *image, uint32_t place_id) {
    so_clear(&image->kitty_gfx);
    so_fmt(&image->kitty_gfx, TUI_ESC_CODE_GOTO(image->dst.anc.x, image->dst.anc.y));
    so_fmt(&image->kitty_gfx, KITTY_GFX_BEGIN "a=p,p=%u,i=%u,x=%u,y=%u,w=%u,h=%u,c=%u,r=%u,z=%u,C=1" KITTY_GFX_END,
            place_id, image->id,
            image->src.anc.x, image->src.anc.y, image->src.dim.x, image->src.dim.y,
            image->dst.dim.x, image->dst.dim.y, image->z);
}

int tui_image_update(struct Tui_Core *core, Tui_Image *image, So *errmsg) {
    int err = 0;
    if(tui_image_is_supported(core)) {
        tui_image_kitty_gfx_send(image);
        err = !tui_input_await_image_data(&core->input_gen.special, image->kitty_gfx);
        if(errmsg) *errmsg = core->input_gen.special.kitty_graphics.message;
#if 0
        if(err) {
            printf("%.*s\r\n", SO_F(core->input_gen.special.kitty_graphics.message));
        }
#endif
    } else {
        err = -1;
    }
    return err;
}

void tui_image_config(Tui_Image *image, Tui_Rect src, Tui_Rect dst, int32_t z) {
    image->src = src;
    image->dst = dst;
    image->z = z;
}

int tui_image_render(struct Tui_Core *core, Tui_Image *image, uint32_t place_id, So *errmsg) {
    int err = 0;
    if(tui_image_is_supported(core)) {
        if(image->dst.anc.x >= core->buffer.dimension.x || image->dst.anc.y >= core->buffer.dimension.y) {
            return 0;
        }
        if(image->dst.dim.x == 0 || image->dst.dim.y == 0) {
            return 0;
        }
        err = 0;
        tui_image_kitty_gfx_place(image, place_id);
        err = !tui_input_await_image_data(&core->input_gen.special, image->kitty_gfx);
        if(errmsg) *errmsg = core->input_gen.special.kitty_graphics.message;
#if 0
        if(err) {
            printf("%.*s\r\n", SO_F(core->input_gen.special.kitty_graphics.message));
        }
#endif
    } else {
        err = -1;
    }
    return err;
}


int tui_image_clear_id_place(struct Tui_Core *core, So *tmp, uint32_t place_id) {
    ASSERT_ARG(core);
    ASSERT_ARG(tmp);
    int err = 0;
    if(tui_image_is_supported(core)) {
        so_clear(tmp);
        so_fmt(tmp, KITTY_GFX_BEGIN "a=d,i=%u" KITTY_GFX_END, place_id);
        tui_write_nstr(tmp->str, tmp->len);
#if 0
        if(err) {
            printf("%.*s\r\n", SO_F(core->input_gen.special.kitty_graphics.message));
        }
#endif
    } else {
        err = -1;
    }
    return err;
}

int tui_image_clear_id_image(struct Tui_Core *core, So *tmp, uint32_t place_id) {
    ASSERT_ARG(core);
    ASSERT_ARG(tmp);
    int err = 0;
    if(tui_image_is_supported(core)) {
        so_clear(tmp);
        so_fmt(tmp, KITTY_GFX_BEGIN "a=D,i=%u" KITTY_GFX_END, place_id);
        tui_write_nstr(tmp->str, tmp->len);
        //err = !tui_input_await_image_data(&core->input_gen.special.kitty_graphics, *tmp);
    } else {
        err = -1;
    }
    return err;
}

