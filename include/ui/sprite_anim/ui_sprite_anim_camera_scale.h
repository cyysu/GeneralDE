#ifndef UI_SPRITE_ANIM_CAMERA_SCALE_H
#define UI_SPRITE_ANIM_CAMERA_SCALE_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_ANIM_CAMERA_SCALE_NAME;

ui_sprite_anim_camera_scale_t ui_sprite_anim_camera_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_anim_camera_scale_free(ui_sprite_anim_camera_scale_t scale);

int8_t ui_sprite_anim_camera_scale_priority(ui_sprite_anim_camera_scale_t scale);
void ui_sprite_anim_camera_scale_set_priority(ui_sprite_anim_camera_scale_t scale, int8_t priority);

#ifdef __cplusplus
}
#endif

#endif
