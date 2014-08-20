#ifndef UI_SPRITE_ANIM_CAMERA_TOUCH_I_H
#define UI_SPRITE_ANIM_CAMERA_TOUCH_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera_touch.h"
#include "ui_sprite_anim_camera_updator.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ui_sprite_anim_camera_touch_state {
    ui_sprite_anim_camera_touch_state_idle
    , ui_sprite_anim_camera_touch_state_move
    , ui_sprite_anim_camera_touch_state_move_by_speed
};

struct ui_sprite_anim_camera_touch {
    ui_sprite_anim_module_t m_module;
    struct ui_sprite_anim_camera_updator m_updator;
    enum ui_sprite_anim_camera_touch_state m_state;
    UI_SPRITE_2D_RECT m_init_camera_rect;
};

int ui_sprite_anim_camera_touch_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_camera_touch_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
