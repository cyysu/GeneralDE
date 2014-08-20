#ifndef UI_SPRITE_2D_SCALE_I_H
#define UI_SPRITE_2D_SCALE_I_H
#include "ui_sprite_2d_module_i.h"
#include "ui/utils/ui_percent_decorator.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_2d_scale {
	ui_sprite_2d_module_t	m_module;
	float					m_max_speed;
	UI_SPRITE_2D_PAIR		m_target_scale;
	UI_SPRITE_2D_PAIR		m_origin_scale;
	float					m_work_duration;
    struct ui_percent_decorator m_decorator;
    float                   m_runing_time;
};

int ui_sprite_2d_scale_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_scale_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
