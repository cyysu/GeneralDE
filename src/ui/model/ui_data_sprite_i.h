#ifndef UI_DATA_SPRITE_INTERNAL_H
#define UI_DATA_SPRITE_INTERNAL_H
#include "ui/model/ui_data_sprite.h"
#include "ui_model_internal_types.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_data_sprite_use * ui_data_sprite_use_t;

typedef TAILQ_HEAD(ui_data_sprite_use_list, ui_data_sprite_use) ui_data_sprite_use_list_t;
typedef TAILQ_HEAD(ui_data_frame_list, ui_data_frame) ui_data_frame_list_t;
typedef TAILQ_HEAD(ui_data_frame_img_list, ui_data_frame_img) ui_data_frame_img_list_t;

struct ui_data_sprite {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    ui_data_frame_list_t m_frames;
    ui_data_sprite_use_list_t m_uses;
};

struct ui_data_sprite_use {
    ui_data_src_ref_t m_src_ref;
    ui_data_frame_img_list_t m_user_img_refs;
    TAILQ_ENTRY(ui_data_sprite_use) m_next_for_sprite;
};

struct ui_data_frame {
    ui_data_sprite_t m_sprite;
    struct cpe_hash_entry m_hh_for_mgr;
    TAILQ_ENTRY(ui_data_frame) m_next_for_sprite;
    ui_data_frame_img_list_t m_img_refs;
    UI_FRAME m_data;
};

struct ui_data_frame_img {
    ui_data_frame_t m_frame;
    ui_data_sprite_use_t m_sprite_use;
    TAILQ_ENTRY(ui_data_frame_img) m_next_for_frame;
    TAILQ_ENTRY(ui_data_frame_img) m_next_for_use;
    UI_IMG_REF m_data;
};

void ui_data_sprite_use_remove_img(ui_data_frame_img_t img_ref);
int ui_data_sprite_use_add_img(ui_data_frame_img_t img_ref);

#ifdef __cplusplus
}
#endif

#endif
