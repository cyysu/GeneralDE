#ifndef UI_SPRITE_ANIM_CAMERA_OP_I_H
#define UI_SPRITE_ANIM_CAMERA_OP_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera_op.h"
#include "ui_sprite_anim_camera_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_camera_op {
    ui_sprite_anim_camera_t m_camera;
    uint32_t m_op_id;
    UI_SPRITE_ANIM_CAMERA_OP m_op;
    uint8_t m_is_done;
    int8_t m_priority;
    ui_sprite_anim_camera_op_suspend_policy_t m_suspend_policy;
    ui_sprite_anim_camera_op_complete_policy_t m_complete_policy;
    TAILQ_ENTRY(ui_sprite_anim_camera_op) m_next_for_camera;
};

void ui_sprite_anim_camera_op_free(ui_sprite_anim_camera_op_t camera_op);
ui_sprite_anim_camera_op_t ui_sprite_anim_camera_op_find(ui_sprite_anim_camera_t camera, uint32_t op_id);

void ui_sprite_anim_camera_op_update_move_to_target(ui_sprite_anim_camera_op_t camera_op, float delta);
void ui_sprite_anim_camera_op_update_move_by_speed(ui_sprite_anim_camera_op_t camera_op, float delta);

#ifdef __cplusplus
}
#endif

#endif
