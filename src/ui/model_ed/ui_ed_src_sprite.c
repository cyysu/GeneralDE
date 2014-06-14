#include <assert.h>
#include "ui/model/ui_data_src.h"
#include "ui/model/ui_data_sprite.h"
#include "ui_ed_src_i.h"
#include "ui_ed_obj_i.h"
#include "ui_ed_rel_i.h"

int ui_ed_src_load_sprite(ui_ed_src_t ed_src) {
    ui_data_sprite_t sprite;
    struct ui_data_frame_it frame_it;
    ui_data_frame_t frame;

    sprite = ui_data_src_product(ed_src->m_data_src);
    assert(sprite);

    ui_data_sprite_frames(&frame_it, sprite);
    while((frame = ui_data_frame_it_next(&frame_it))) {
        struct ui_data_frame_img_it img_it;
        ui_data_frame_img_t img;
        ui_ed_obj_t obj_frame;

        obj_frame = ui_ed_obj_create_i(
            ed_src, ed_src->m_root_obj,
            ui_ed_obj_frame,
            frame, ui_data_frame_data(frame), sizeof(*ui_data_frame_data(frame)));
        if (obj_frame == NULL) continue;

        ui_data_frame_imgs(&img_it, frame);
        while((img = ui_data_frame_img_it_next(&img_it))) {
            ui_ed_obj_create_i(
                ed_src, obj_frame,
                ui_ed_obj_frame_img,
                img, ui_data_frame_img_data(img), sizeof(*ui_data_frame_img_data(img)));
        }
    }

    return 0;
}

ui_ed_obj_t ui_ed_frame_img_load_rel_img(ui_ed_obj_t from) {
    ui_ed_mgr_t ed_mgr = from->m_src->m_ed_mgr;
    UI_IMG_REF * img_ref;
    ui_ed_src_t src;

    img_ref = from->m_data;
    assert(img_ref);

    src = ui_ed_src_find_by_id(ed_mgr, img_ref->module_id);
    if (src == NULL) return NULL;

    if (ui_ed_src_load(src, ed_mgr->m_em) != 0) return NULL;

    return ui_ed_obj_find_by_id(src, img_ref->img_block_id);
}
