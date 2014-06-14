#ifndef UI_SPRITE_ANIM_CAMERA_OP_H
#define UI_SPRITE_ANIM_CAMERA_OP_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_sprite_anim_camera_op_suspend_policy {
    ui_sprite_anim_camera_op_suspend_suspend = 1,
    ui_sprite_anim_camera_op_suspend_remove = 2
} ui_sprite_anim_camera_op_suspend_policy_t;

typedef enum ui_sprite_anim_camera_op_complete_policy {
    ui_sprite_anim_camera_op_complete_remove = 1,
    ui_sprite_anim_camera_op_complete_keep = 2
} ui_sprite_anim_camera_op_complete_policy_t;

uint32_t
ui_sprite_anim_camera_op_start(
    ui_sprite_anim_camera_t camera,
    UI_SPRITE_ANIM_CAMERA_OP const * op,
    int8_t priority,
    ui_sprite_anim_camera_op_suspend_policy_t suspend_policy,
    ui_sprite_anim_camera_op_complete_policy_t complete_policy);

uint32_t
ui_sprite_anim_camera_op_check_start(
    ui_sprite_anim_camera_t camera,
    uint32_t op_id,
    UI_SPRITE_ANIM_CAMERA_OP const * op,
    int8_t priority,
    ui_sprite_anim_camera_op_suspend_policy_t suspend_policy,
    ui_sprite_anim_camera_op_complete_policy_t complete_policy);

void ui_sprite_anim_camera_op_stop(ui_sprite_anim_camera_t camera, uint32_t op_id);

uint8_t ui_sprite_anim_camera_op_is_runing(ui_sprite_anim_camera_t camera, uint32_t op_id);

uint32_t ui_sprite_anim_camera_op_curent(ui_sprite_anim_camera_t camera);

#ifdef __cplusplus
}
#endif

#endif
