#ifndef UI_SPRITE_ANIM_CAMERA_MOVE_H
#define UI_SPRITE_ANIM_CAMERA_MOVE_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_ANIM_CAMERA_MOVE_NAME;

ui_sprite_anim_camera_move_t ui_sprite_anim_camera_move_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_anim_camera_move_free(ui_sprite_anim_camera_move_t move);

int8_t ui_sprite_anim_camera_move_priority(ui_sprite_anim_camera_move_t move);
void ui_sprite_anim_camera_move_set_priority(ui_sprite_anim_camera_move_t move, int8_t priority);

#ifdef __cplusplus
}
#endif

#endif
