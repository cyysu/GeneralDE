#ifndef UI_SPRITE_ANIM_CAMERA_TOUCH_I_H
#define UI_SPRITE_ANIM_CAMERA_TOUCH_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera_touch.h"
#include "ui_sprite_anim_camera_op_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_camera_touch {
    ui_sprite_anim_module_t m_module;
    uint32_t m_curent_op_id;
    int8_t m_priority;
};

int ui_sprite_anim_camera_touch_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_camera_touch_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
