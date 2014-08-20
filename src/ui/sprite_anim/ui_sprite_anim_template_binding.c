#include <assert.h>
#include "cpe/dr/dr_data.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_anim_template_binding_i.h"
#include "ui_sprite_anim_template_i.h"
#include "ui_sprite_anim_sch_i.h"
#include "ui_sprite_anim_backend_i.h"
#include "ui_sprite_anim_module_i.h"

ui_sprite_anim_template_binding_t
ui_sprite_anim_template_binding_create(
    ui_sprite_anim_template_t tpl, const char * ctrl_name, const char * attr_name, const char * attr_value)
{
    ui_sprite_anim_sch_t anim_sch = tpl->m_anim_sch;
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;

    size_t ctrl_name_len = strlen(ctrl_name) + 1;
    size_t attr_name_len = strlen(attr_name) + 1;
    size_t attr_value_len = strlen(attr_value) + 1;
    char * p;

    ui_sprite_anim_template_binding_t binding;

    binding = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_anim_template_binding) + ctrl_name_len + attr_name_len + attr_value_len);
    if (binding == NULL) {
        CPE_ERROR(
            module->m_em, "anim template %s: add binding %s.%s = %s: alloc fail!",
            tpl->m_name, ctrl_name, attr_name, attr_value);
        return NULL;
    }

    p = (void*)(binding + 1);

    memcpy(p, ctrl_name, ctrl_name_len);
    binding->m_control = p;
    p += ctrl_name_len;

    memcpy(p, attr_name, attr_name_len);
    binding->m_attr_name = p;
    p += attr_name_len;

    memcpy(p, attr_value, attr_value_len);
    binding->m_attr_value = p;
    p += attr_value_len;

    TAILQ_INSERT_TAIL(&tpl->m_bindings, binding, m_next_for_template);

    return binding;
}

void ui_sprite_anim_template_binding_free(ui_sprite_anim_template_t tpl, ui_sprite_anim_template_binding_t binding) {
    ui_sprite_anim_sch_t anim_sch = tpl->m_anim_sch;
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;

    TAILQ_REMOVE(&tpl->m_bindings, binding, m_next_for_template);

    mem_free(module->m_alloc, binding);
}

int ui_sprite_anim_template_binding_set_value(
    ui_sprite_anim_template_binding_t binding, ui_sprite_anim_template_t tpl, uint32_t anim_id, ui_sprite_entity_t entity)
{
    ui_sprite_anim_module_t module = tpl->m_anim_sch->m_backend->m_module;
    ui_sprite_anim_backend_t backend = tpl->m_anim_sch->m_backend;
    const char * attr_value;

    attr_value = ui_sprite_entity_try_calc_str(&module->m_dump_buffer, binding->m_attr_value, entity, NULL, module->m_em);
    if (attr_value == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): template %s: attr %s.%s: calc %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), tpl->m_name,
            binding->m_control, binding->m_attr_name, binding->m_attr_value);
        return -1;
    }

    if (backend->m_def.m_set_template_value(
            backend->m_def.m_ctx, anim_id,
            binding->m_control, binding->m_attr_name, attr_value)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): template %s: attr %s.%s: set value %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), tpl->m_name,
            binding->m_control, binding->m_attr_name, attr_value);
        return -1;
    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): template %s: attr %s.%s ==> %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), tpl->m_name,
            binding->m_control, binding->m_attr_name, attr_value);
    }

    return 0;
}
