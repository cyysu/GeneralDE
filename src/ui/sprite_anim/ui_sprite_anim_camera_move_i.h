#ifndef UI_SPRITE_ANIM_CAMERA_MOVE_I_H
#define UI_SPRITE_ANIM_CAMERA_MOVE_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera_move.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_camera_move {
    ui_sprite_anim_module_t m_module;
    uint32_t m_curent_op_id;
    int8_t m_priority;
    uint32_t m_follow_entity_id;
    char m_follow_entity_name[64];
    uint8_t m_follow_entity_pos;
    UI_SPRITE_2D_PAIR m_pos_on_screen;
    float m_target_scale;
    float m_max_speed;
    float m_duration;
};

int ui_sprite_anim_camera_move_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_camera_move_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
