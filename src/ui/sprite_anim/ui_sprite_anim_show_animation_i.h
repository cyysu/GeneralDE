#ifndef UI_SPRITE_ANIM_SHOW_ANIMATION_I_H
#define UI_SPRITE_ANIM_SHOW_ANIMATION_I_H
#include "ui/sprite_anim/ui_sprite_anim_show_animation.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_show_animation {
    ui_sprite_anim_module_t m_module;
    uint32_t m_anim_id;
    char m_res[128];
    char m_group[64];
    uint8_t m_loop;
    int32_t m_start;
    int32_t m_end;
};

int ui_sprite_anim_show_animation_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_show_animation_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
