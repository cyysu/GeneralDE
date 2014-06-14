#ifndef UI_SPRITE_ANIM_TEMPLATE_BINDING_I_H
#define UI_SPRITE_ANIM_TEMPLATE_BINDING_I_H
#include "ui/sprite_anim/ui_sprite_anim_template_binding.h"
#include "ui_sprite_anim_template_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ui_sprite_anim_template_binding_attr_type {
    ui_sprite_anim_template_binding_attr_type_value,
    ui_sprite_anim_template_binding_attr_type_attr
};

struct ui_sprite_anim_template_binding {
    const char * m_control;
    const char * m_attr_name;
    const char * m_attr_value;
    enum ui_sprite_anim_template_binding_attr_type m_attr_type;
    TAILQ_ENTRY(ui_sprite_anim_template_binding) m_next_for_template;
};

void ui_sprite_anim_template_binding_free(ui_sprite_anim_template_t tmpl, ui_sprite_anim_template_binding_t binding);

int ui_sprite_anim_template_binding_set_value(
    ui_sprite_anim_template_binding_t binding, ui_sprite_anim_template_t tpl, uint32_t anim_id, ui_sprite_entity_t entity);

#ifdef __cplusplus
}
#endif

#endif
