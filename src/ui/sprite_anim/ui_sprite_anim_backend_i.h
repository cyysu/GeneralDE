#ifndef UI_SPRITE_ANIM_BACKEND_I_H
#define UI_SPRITE_ANIM_BACKEND_I_H
#include "ui/sprite_anim/ui_sprite_anim_backend.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_backend {
    ui_sprite_anim_module_t m_module;
    size_t m_size;
    ui_sprite_anim_runing_list_t m_runing_anims;

    /*extern functions*/
    struct ui_sprite_anim_backend_def m_def;
};

#ifdef __cplusplus
}
#endif

#endif
