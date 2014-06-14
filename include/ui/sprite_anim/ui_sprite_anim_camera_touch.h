#ifndef UI_SPRITE_ANIM_CAMERA_ACTION_TOUCH_H
#define UI_SPRITE_ANIM_CAMERA_ACTION_TOUCH_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_ANIM_CAMERA_TOUCH_NAME;

ui_sprite_anim_camera_touch_t ui_sprite_anim_camera_touch_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_anim_camera_touch_free(ui_sprite_anim_camera_touch_t touch);

int8_t ui_sprite_anim_camera_touch_priority(ui_sprite_anim_camera_touch_t touch);
void ui_sprite_anim_camera_touch_set_priority(ui_sprite_anim_camera_touch_t touch, int8_t priority);

#ifdef __cplusplus
}
#endif

#endif
