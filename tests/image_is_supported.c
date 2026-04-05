#include "../rltui.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

struct Tui_Core *core;

int image_ch;
Tui_Point image_dim;
So image_raw;
So tmpbuf;
Tui_Image *image_tui;

bool input(Tui_Input *input, bool *flush, void *user) {
    bool supported = tui_image_is_supported(core);
    //usleep(1e2);
    //printf("image support: %u\r\n", supported);
    if(input->id == INPUT_TEXT) {
        if(input->text.val == 'q') {
            tui_core_quit(core);
            return false;
        } else if(input->text.val == 'L') {
            if(!image_tui) {

                FILE *fp = so_file_fp(so("../ctm/assets/gentoo.png"), "r");
                image_raw.str = (char *)stbi_load_from_file(fp, (int *)&image_dim.x, (int *)&image_dim.y, &image_ch, 0);
                //printff("data %p %ux%ux%u\r\n",data,image_dim.x,image_dim.y,image_ch);

                image_tui = tui_image_new(core, 10, (uint8_t *)image_raw.str, image_dim, image_ch);
                int er = tui_image_update(core, image_tui, 0);
                tui_image_config(image_tui, (Tui_Rect){ .dim = image_dim }, (Tui_Rect){ .dim.x = 10, .dim.y = 5 }, 1);
                //printf("image load / update err : %u\r\n",er);
            }
            return true;
        } else if(input->text.val == 'l') {
            ++image_tui->dst.anc.x;
        } else if(input->text.val == 'j') {
            ++image_tui->dst.anc.y;
        } else if(input->text.val == 'k') {
            --image_tui->dst.anc.y;
        } else if(input->text.val == 'h') {
            --image_tui->dst.anc.x;
        } else if(input->text.val == '+') {
            ++image_tui->dst.dim.x;
            ++image_tui->dst.dim.x;
            ++image_tui->dst.dim.y;
        } else if(input->text.val == '-') {
            --image_tui->dst.dim.x;
            --image_tui->dst.dim.x;
            --image_tui->dst.dim.y;
        } else if(input->text.val == 'c') {
            image_tui = 0;
            return true;
        }
    }
    return true;
}

bool update(void *user) {
    return false;
}

void render(Tui_Buffer *buffer, void *user) {
    So errmsg = SO;
    if(image_tui) {
        int er = tui_image_render(core, image_tui, 20, &errmsg);
        //printf("image display err : %u '%.*s'\r\n",er, SO_F(errmsg));
        //usleep(1e6);
    } else if(image_raw.str) {
        int er = tui_image_clear_id_place(core, image_tui->id, 20);
    }
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

