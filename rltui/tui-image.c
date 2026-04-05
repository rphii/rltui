#include <stdbool.h>
#include "tui-image.h"
#include "tui-core-internal.h"
#include "tui-write.h"

#define KITTY_GFX_BEGIN     "\e_G"
#define KITTY_GFX_END       "\e\\"

bool tui_image_is_supported(struct Tui_Core *core) {
    if(!core->is_graphics_support_queried) {
        core->is_graphics_support_ok = tui_input_await_image_support(core);
        core->is_graphics_support_queried = true;
    }
    return core->is_graphics_support_ok;
}

Tui_Image *tui_image_new(struct Tui_Core *core, uint32_t id, uint8_t *data, Tui_Point dimensions, int channels) {
    Tui_Image *img;
    NEW(Tui_Image, img);
    pthread_mutex_lock(&core->input_gen.special.mtx);
    array_push(core->images, img);
    img->channels = channels;
    img->data = data;
    img->dimensions = dimensions;
    img->id = id;
    pthread_mutex_unlock(&core->input_gen.special.mtx);
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

void tui_image_kitty_gfx_place(Tui_Image *image, uint32_t place_id, Tui_Point anc_shift_px, Tui_Point dst_dim, Tui_Point goto_xy) {
    so_clear(&image->kitty_gfx);
    //anc_shift_px = (Tui_Point){0};
    so_fmt(&image->kitty_gfx, TUI_ESC_CODE_GOTO(goto_xy.x, goto_xy.y));
    so_fmt(&image->kitty_gfx, KITTY_GFX_BEGIN "a=p,p=%u,i=%u,s=%u,v=%u,x=%u,y=%u,w=%u,h=%u,c=%u,r=%u,z=%u,C=1" KITTY_GFX_END,
            place_id, image->id, image->dimensions.x, image->dimensions.y,
            image->src.anc.x + anc_shift_px.x, image->src.anc.y + anc_shift_px.y, image->src.dim.x - anc_shift_px.x, image->src.dim.y - anc_shift_px.y,
            dst_dim.x, dst_dim.y, image->z);
}

int tui_image_update(struct Tui_Core *core, Tui_Image *image, So *errmsg) {
    int err = 0;
    if(tui_image_is_supported(core)) {
        tui_image_kitty_gfx_send(image);
        err = !tui_input_await_image_data(core, image->kitty_gfx);
        if(errmsg) *errmsg = core->input_gen.special.kitty_graphics.message;
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
    if(!image) return 0;

    Tui_Point goto_xy = image->dst.anc;
    Tui_Point dst_dim = image->dst.dim;
    Tui_Point shift_px = {0};
    double ratio_y = 0, ratio_x = 0;

    if(tui_image_is_supported(core)) {
        if(image->dst.anc.x >= core->buffer.dimension.x || image->dst.anc.y >= core->buffer.dimension.y) {
            tui_image_clear_id_place(core, image->id, place_id);
            return 0;
        }
        if(image->dst.dim.x <= 0 || image->dst.dim.y <= 0) {
            tui_image_clear_id_place(core, image->id, place_id);
            return 0;
        }

        /* if going out of bounds, but still can display image, correct offets */
        if(image->dst.anc.x < 0) {
            ssize_t n = image->dst.anc.x + image->dst.dim.x;
            if(n < 0) {
                tui_image_clear_id_place(core, image->id, place_id);
                return 0;
            }
            ratio_x = (double)image->src.dim.x / (double)image->dst.dim.x;
            shift_px.x = (dst_dim.x - n) * ratio_x;
            goto_xy.x = 0;
            dst_dim.x = n;
        }

        /* if going out of bounds, but still can display image, correct offets */
        if(image->dst.anc.y < 0) {
            ssize_t n = image->dst.anc.y + image->dst.dim.y;
            if(n < 0) {
                tui_image_clear_id_place(core, image->id, place_id);
                return 0;
            }
            ratio_y = (double)image->src.dim.y / (double)image->dst.dim.y;
            shift_px.y = (dst_dim.y - n) * ratio_y;
            goto_xy.y = 0;
            dst_dim.y = n;
        }

        err = 0;
        tui_image_kitty_gfx_place(image, place_id, shift_px, dst_dim, goto_xy);
        err = !tui_input_await_image_data(core, image->kitty_gfx);
        if(errmsg) *errmsg = core->input_gen.special.kitty_graphics.message;
    } else {
        //printff("UNSOPPORTED");
        err = -1;
    }
    return err;
}


int tui_image_clear_id_place(struct Tui_Core *core, uint32_t image_id, uint32_t place_id) {
    ASSERT_ARG(core);
    int err = 0;
    if(tui_image_is_supported(core)) {
        pthread_mutex_lock(&core->mtx_tmp);
        so_clear(&core->tmp);
        so_fmt(&core->tmp, KITTY_GFX_BEGIN "a=d,d=i,i=%u,p=%u" KITTY_GFX_END, image_id, place_id);
        tui_core_write(core, core->tmp);
        pthread_mutex_unlock(&core->mtx_tmp);
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

int tui_image_clear_id_image(struct Tui_Core *core, uint32_t image_id) {
    ASSERT_ARG(core);
    int err = 0;
    if(tui_image_is_supported(core)) {
        pthread_mutex_lock(&core->mtx_tmp);
        so_clear(&core->tmp);
        so_fmt(&core->tmp, KITTY_GFX_BEGIN "a=d,d=i,i=%u" KITTY_GFX_END, image_id);
        tui_core_write(core, core->tmp);
        pthread_mutex_unlock(&core->mtx_tmp);
        //err = !tui_input_await_image_data(&core->input_gen.special.kitty_graphics, *tmp);
    } else {
        err = -1;
    }
    return err;
}

int tui_image_clear_all(struct Tui_Core *core) {
    ASSERT_ARG(core);
    int err = 0;
    if(tui_image_is_supported(core)) {
        tui_core_write(core, so("a=d"));
        //err = !tui_input_await_image_data(&core->input_gen.special.kitty_graphics, *tmp);
    } else {
        err = -1;
    }
    return err;
}

