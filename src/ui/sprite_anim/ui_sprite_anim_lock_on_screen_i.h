#ifndef UI_SPRITE_ANIM_LOCK_ON_SCREEN_I_H
#define UI_SPRITE_ANIM_LOCK_ON_SCREEN_I_H
#include "ui/utils/ui_percent_decorator.h"
#include "ui/sprite_anim/ui_sprite_anim_lock_on_screen.h"
#include "ui_sprite_anim_camera_updator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_lock_on_screen {
    ui_sprite_anim_module_t m_module;
    struct ui_sprite_anim_camera_updator m_updator;
    UI_SPRITE_2D_PAIR m_pos_on_screen;
    float m_scale;
    struct ui_percent_decorator m_decorator;
    float m_max_speed;

    UI_SPRITE_2D_PAIR m_init_pos_on_screen;
    float m_runing_time;
    float m_duration;
};

int ui_sprite_anim_lock_on_screen_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_lock_on_screen_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
