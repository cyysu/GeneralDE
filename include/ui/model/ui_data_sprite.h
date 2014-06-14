#ifndef UI_MODEL_DATA_SPRITE_H
#define UI_MODEL_DATA_SPRITE_H
#include "protocol/ui/model/ui_sprite.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_frame_it {
    ui_data_frame_t (*next)(struct ui_data_frame_it * it);
    char m_data[64];
};

struct ui_data_frame_img_it {
    ui_data_frame_img_t (*next)(struct ui_data_frame_img_it * it);
    char m_data[64];
};

/*sprite */
ui_data_sprite_t ui_data_sprite_create(ui_data_mgr_t mgr, ui_data_src_t src);
void ui_data_sprite_free(ui_data_sprite_t sprite);
void ui_data_sprite_frames(ui_data_frame_it_t it, ui_data_sprite_t sprite);
int ui_data_sprite_update_refs(ui_data_sprite_t sprite);
void ui_data_sprite_use_srcs(ui_data_src_it_t it, ui_data_sprite_t sprite);
void ui_data_sprite_use_refs(ui_data_src_ref_it_t it, ui_data_sprite_t sprite);

/*frame*/
ui_data_frame_t ui_data_frame_create(ui_data_sprite_t sprite, uint32_t id);
void ui_data_frame_free(ui_data_frame_t frame);
void ui_data_frame_imgs(ui_data_frame_img_it_t it, ui_data_frame_t frame);
UI_FRAME * ui_data_frame_data(ui_data_frame_t frame);
LPDRMETA ui_data_frame_meta(ui_data_mgr_t mgr);

#define ui_data_frame_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*img_ref*/
ui_data_frame_img_t ui_data_frame_img_create(ui_data_frame_t frame);
void ui_data_frame_img_free(ui_data_frame_img_t img_ref);
UI_IMG_REF * ui_data_frame_img_data(ui_data_frame_img_t img_ref);
LPDRMETA ui_data_frame_img_meta(ui_data_mgr_t mgr);

int ui_data_frame_img_update_ref(ui_data_frame_img_t img_ref);

#define ui_data_frame_img_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
