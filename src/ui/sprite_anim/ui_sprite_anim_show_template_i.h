#ifndef UI_SPRITE_ANIM_SHOW_TEMPLATE_I_H
#define UI_SPRITE_ANIM_SHOW_TEMPLATE_I_H
#include "ui/sprite_anim/ui_sprite_anim_show_template.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_anim_show_template_binding * ui_sprite_anim_show_template_binding_t;
typedef TAILQ_HEAD(ui_sprite_anim_show_template_binding_list, ui_sprite_anim_show_template_binding) ui_sprite_anim_show_template_binding_list_t;

struct ui_sprite_anim_show_template {
    ui_sprite_anim_module_t m_module;
    uint32_t m_anim_id;
    char m_template[128];
    ui_sprite_anim_show_template_binding_list_t m_bindings;
};

int ui_sprite_anim_show_template_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_show_template_unregist(ui_sprite_anim_module_t module);

struct ui_sprite_anim_show_template_binding {
    ui_sprite_anim_show_template_t m_show_template;
    ui_sprite_anim_template_binding_t m_binding;
    TAILQ_ENTRY(ui_sprite_anim_show_template_binding) m_next;
};

ui_sprite_anim_show_template_binding_t
ui_sprite_anim_show_template_binding_create(
    ui_sprite_anim_show_template_t show_template, ui_sprite_anim_template_binding_t binding);

void ui_sprite_anim_show_template_binding_free(ui_sprite_anim_show_template_binding_t binding);

#ifdef __cplusplus
}
#endif

#endif
