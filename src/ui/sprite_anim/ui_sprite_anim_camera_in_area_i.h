#ifndef UI_SPRITE_ANIM_CAMERA_IN_AREA_I_H
#define UI_SPRITE_ANIM_CAMERA_IN_AREA_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera_in_area.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_camera_in_area {
	ui_sprite_anim_module_t m_module;
	ui_sprite_anim_camera_restrict_t m_restrict;
};
	
int ui_sprite_anim_camera_in_area_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_camera_in_area_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
