#ifndef UI_SPRITE_ANIM_CAMERA_IN_AREA_H
#define UI_SPRITE_ANIM_CAMERA_IN_AREA_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern const char * UI_SPRITE_ANIM_CAMERA_IN_AREA_NAME;

	ui_sprite_anim_camera_in_area_t ui_sprite_anim_camera_in_area_create(ui_sprite_fsm_state_t fsm_state, const char * name);
	void ui_sprite_anim_camera_in_area_free(ui_sprite_anim_camera_in_area_t inarea);

	int8_t ui_sprite_anim_camera_in_area_priority(ui_sprite_anim_camera_in_area_t inarea);
	void ui_sprite_anim_camera_in_area_set_priority(ui_sprite_anim_camera_in_area_t inarea, int8_t priority);

#ifdef __cplusplus
}
#endif

#endif
