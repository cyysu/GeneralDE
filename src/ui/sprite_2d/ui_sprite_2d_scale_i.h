#ifndef UI_SPRITE_2D_SCALE_I_H
#define UI_SPRITE_2D_SCALE_I_H
#include "ui_sprite_2d_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_2d_scale {
	ui_sprite_2d_module_t	m_module;
	float					m_max_speed;
	UI_SPRITE_2D_PAIR		m_scale;
	UI_SPRITE_2D_PAIR		m_scale_speed;
	float					m_work_duration;
};



int ui_sprite_2d_scale_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_scale_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
