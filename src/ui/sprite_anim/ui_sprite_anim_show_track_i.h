#ifndef UI_SPRITE_ANIM_SHOW_TRACK_I_H
#define UI_SPRITE_ANIM_SHOW_TRACK_I_H
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_anim_show_track {
	ui_sprite_anim_module_t m_module;

    uint8_t m_track_pos;
    UI_SPRITE_2D_PAIR m_pre_pos;
};

int ui_sprite_anim_show_track_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_show_track_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
