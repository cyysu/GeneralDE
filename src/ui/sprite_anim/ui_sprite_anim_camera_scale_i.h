#ifndef UI_SPRITE_ANIM_CAMERA_SCALE_I_H
#define UI_SPRITE_ANIM_CAMERA_SCALE_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera_scale.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_camera_scale {
    ui_sprite_anim_module_t m_module;
    uint32_t m_curent_op_id;
    int8_t m_priority;
    float m_target_scale;
    UI_SPRITE_2D_PAIR m_pos_in_world;
    UI_SPRITE_2D_PAIR m_pos_on_screen;
    float m_duration;
};

int ui_sprite_anim_camera_scale_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_camera_scale_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
