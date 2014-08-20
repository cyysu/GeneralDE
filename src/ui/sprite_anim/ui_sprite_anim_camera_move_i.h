#ifndef UI_SPRITE_ANIM_CAMERA_MOVE_I_H
#define UI_SPRITE_ANIM_CAMERA_MOVE_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera_move.h"
#include "ui_sprite_anim_module_i.h"
#include "ui_sprite_anim_camera_updator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_camera_move {
    ui_sprite_anim_module_t m_module;
    struct ui_sprite_anim_camera_updator m_updator;

    UI_SPRITE_2D_PAIR m_pos_in_world;
    UI_SPRITE_2D_PAIR m_pos_in_screen;
};

int ui_sprite_anim_camera_move_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_camera_move_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
