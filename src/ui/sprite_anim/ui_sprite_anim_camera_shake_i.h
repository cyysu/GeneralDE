#ifndef UI_SPRITE_ANIM_CAMERA_SHAKE_I_H
#define UI_SPRITE_ANIM_CAMERA_SHAKE_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera_shake.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_camera_shake {
    ui_sprite_anim_module_t m_module;
	UI_SPRITE_2D_PAIR m_speed;
	uint32_t m_num;
	float m_onceDuration;
	float m_work_duration;
	UI_SPRITE_2D_PAIR m_camera_pos;
	/* Õñ·ù¼ÆÊý */
	uint8_t m_amplitude_count;
};

int ui_sprite_anim_camera_shake_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_camera_shake_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
