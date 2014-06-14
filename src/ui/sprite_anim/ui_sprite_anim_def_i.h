#ifndef UI_SPRITE_ANIM_DEF_I_H
#define UI_SPRITE_ANIM_DEF_I_H
#include "ui/sprite_anim/ui_sprite_anim_def.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_def {
    ui_sprite_anim_sch_t m_anim_sch;
    uint8_t m_auto_start;
    const char * m_anim_name;
    const char * m_anim_res;

    TAILQ_ENTRY(ui_sprite_anim_def) m_next_for_backend;
    TAILQ_ENTRY(ui_sprite_anim_def) m_next_for_sch;
};

#ifdef __cplusplus
}
#endif

#endif
