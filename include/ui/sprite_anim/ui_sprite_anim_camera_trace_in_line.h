#ifndef UI_SPRITE_ANIM_CAMERA_TRACE_IN_LINE_H
#define UI_SPRITE_ANIM_CAMERA_TRACE_IN_LINE_H
#include "ui_sprite_anim_types.h"
#include "ui/sprite_anim/ui_sprite_anim_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_ANIM_CAMERA_TRACE_IN_LINE_NAME;

ui_sprite_anim_camera_trace_in_line_t ui_sprite_anim_camera_trace_in_line_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_anim_camera_trace_in_line_free(ui_sprite_anim_camera_trace_in_line_t trace_in_line);

#ifdef __cplusplus
}
#endif

#endif