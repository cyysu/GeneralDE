#ifndef UI_SPRITE_ANIM_CAMERA_I_H
#define UI_SPRITE_ANIM_CAMERA_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_camera {
    ui_sprite_anim_module_t m_module;
    uint8_t m_updating;
    UI_SPRITE_2D_PAIR m_screen_size;
    UI_SPRITE_2D_PAIR m_limit_lt;
    UI_SPRITE_2D_PAIR m_limit_rb;
    UI_SPRITE_2D_PAIR m_camera_pos;
    float m_camera_scale;
    UI_SPRITE_2D_PAIR m_camera_scale_pair;
    ui_sprite_anim_camera_restrict_list_t m_restricts;
    uint32_t m_max_op_id;
    ui_sprite_anim_camera_op_list_t m_ops;
};

int ui_sprite_anim_camera_pos_of_entity(UI_SPRITE_2D_PAIR * pos, ui_sprite_world_t world, uint32_t entity_id, const char * entity_name, uint8_t pos_of_entity);
void ui_sprite_anim_camera_set_pos_and_scale_no_adj(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos, float scale);
void ui_sprite_anim_camera_sync_update(ui_sprite_anim_camera_t camera);

#ifdef __cplusplus
}
#endif

#endif
