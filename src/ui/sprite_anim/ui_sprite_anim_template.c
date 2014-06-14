#include <assert.h>
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_anim_template_i.h"
#include "ui_sprite_anim_template_binding_i.h"
#include "ui_sprite_anim_sch_i.h"
#include "ui_sprite_anim_backend_i.h"
#include "ui_sprite_anim_module_i.h"

ui_sprite_anim_template_t ui_sprite_anim_template_create(ui_sprite_anim_sch_t anim_sch, const char * name, const char * group, const char * res) {
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;
    ui_sprite_anim_template_t anim_template;
    size_t name_len = strlen(name) + 1;
    size_t group_len = strlen(group) + 1;
    size_t res_len = strlen(res) + 1;
    char * p;

    anim_template = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_anim_template) + name_len + group_len + res_len);
    if (anim_template == NULL) {
        CPE_ERROR(module->m_em, "crate anim template %s: alloc fail!", name);
        return NULL;
    }

    anim_template->m_anim_sch = anim_sch;

    p = (char *)(anim_template + 1);

    memcpy(p, name, name_len);
    anim_template->m_name = p;
    p += name_len;

    memcpy(p, group, group_len);
    anim_template->m_group = p;
    p += group_len;

    memcpy(p, res, res_len);
    anim_template->m_res = p;
    p += res_len;

    TAILQ_INIT(&anim_template->m_bindings);

    TAILQ_INSERT_TAIL(&anim_sch->m_templates, anim_template, m_next_for_sch);

    return anim_template;
}

ui_sprite_anim_template_t ui_sprite_anim_template_clone(ui_sprite_anim_sch_t anim_sch, ui_sprite_anim_template_t o) {
    ui_sprite_anim_template_t template = ui_sprite_anim_template_create(anim_sch, o->m_name, o->m_group, o->m_res);
    return template;
}

void ui_sprite_anim_template_free(ui_sprite_anim_template_t anim_template) {
    ui_sprite_anim_sch_t anim_sch = anim_template->m_anim_sch;
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;

    while(!TAILQ_EMPTY(&anim_template->m_bindings)) {
        ui_sprite_anim_template_binding_free(anim_template, TAILQ_FIRST(&anim_template->m_bindings));
    }

    TAILQ_REMOVE(&anim_sch->m_templates, anim_template, m_next_for_sch);

    mem_free(module->m_alloc, anim_template);
}

ui_sprite_anim_template_t ui_sprite_anim_template_find(ui_sprite_anim_sch_t anim_sch, const char * name) {
    ui_sprite_anim_template_t anim_template;

    TAILQ_FOREACH(anim_template, &anim_sch->m_templates, m_next_for_sch) {
        if (strcmp(anim_template->m_name, name) == 0) return anim_template;
    }

    return NULL;
}

