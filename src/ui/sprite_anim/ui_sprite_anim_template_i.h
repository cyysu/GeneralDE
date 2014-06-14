#ifndef UI_SPRITE_ANIM_TEMPLATE_I_H
#define UI_SPRITE_ANIM_TEMPLATE_I_H
#include "ui/sprite_anim/ui_sprite_anim_template.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_template {
    ui_sprite_anim_sch_t m_anim_sch;
    const char * m_name;
    const char * m_res;
    const char * m_group;
    ui_sprite_anim_template_binding_list_t m_bindings;
    TAILQ_ENTRY(ui_sprite_anim_template) m_next_for_sch;
};

ui_sprite_anim_template_t ui_sprite_anim_template_clone(ui_sprite_anim_sch_t anim_sch, ui_sprite_anim_template_t o);

#ifdef __cplusplus
}
#endif

#endif
