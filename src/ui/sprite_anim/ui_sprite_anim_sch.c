#include <assert.h>
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_anim_sch_i.h"
#include "ui_sprite_anim_runing_i.h"
#include "ui_sprite_anim_group_i.h"
#include "ui_sprite_anim_template_i.h"
#include "ui_sprite_anim_def_i.h"
#include "ui_sprite_anim_backend_i.h"

ui_sprite_anim_sch_t ui_sprite_anim_sch_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_ANIM_SCH_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

ui_sprite_anim_sch_t ui_sprite_anim_sch_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_ANIM_SCH_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

void ui_sprite_anim_sch_free(ui_sprite_anim_sch_t anim_sch) {
    ui_sprite_component_t component = ui_sprite_component_from_data(anim_sch);
    if (component) {
        ui_sprite_component_free(component);
    }
}

uint32_t ui_sprite_anim_sch_start_anim(
    ui_sprite_anim_sch_t anim_sch, const char * group_name,
    const char * res, uint8_t is_loop, int32_t start, int32_t end)
{
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(anim_sch));
    ui_sprite_anim_module_t module = anim_sch->m_backend->m_module;
    ui_sprite_anim_group_t group;
    ui_sprite_anim_runing_t runing;

    group = ui_sprite_anim_group_find_by_name(anim_sch, group_name);
    if (group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): start anim  %s: group %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res, group_name);
        return UI_SPRITE_INVALID_ANIM_ID;
    }

    if (res[0] == '@') {
        ui_sprite_anim_def_t def = ui_sprite_anim_def_find(anim_sch, res + 1);
        if (def == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): start anim %s: anim not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
            return UI_SPRITE_INVALID_ANIM_ID;
        }

        res = def->m_anim_res;
    }

    runing = ui_sprite_anim_runing_create(anim_sch, group, res);
    if (runing == NULL) return UI_SPRITE_INVALID_ANIM_ID;

    return runing->m_anim_id;
}

void ui_sprite_anim_sch_stop_anim(ui_sprite_anim_sch_t anim_sch, uint32_t anim_id) {
    ui_sprite_anim_runing_t runing;

    runing = ui_sprite_anim_runing_find_by_id(anim_sch, anim_id);
    if (runing) {
        ui_sprite_anim_runing_free(runing);
    }
}

uint8_t ui_sprite_anim_sch_is_anim_runing(ui_sprite_anim_sch_t anim_sch, uint32_t anim_id) {
    ui_sprite_anim_runing_t runing;

    runing = ui_sprite_anim_runing_find_by_id(anim_sch, anim_id);

    if (runing == NULL) return 0;

    return anim_sch->m_backend->m_def.m_is_runing_fun(anim_sch->m_backend->m_def.m_ctx, runing->m_anim_id);
}

static void ui_sprite_anim_sch_on_transform_update(void * ctx) {
    ui_sprite_anim_sch_t anim_sch = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(anim_sch));
    ui_sprite_anim_group_t group;
    ui_sprite_2d_transform_t transform;

    transform = ui_sprite_2d_transform_find(entity);

    TAILQ_FOREACH(group, &anim_sch->m_groups, m_next_for_sch) {
        ui_sprite_anim_group_update(group, transform);
    }
}

static int ui_sprite_anim_sch_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_sch_t sch = ui_sprite_component_data(component);

    ui_sprite_anim_backend_t backend = ui_sprite_anim_backend_find(world);
    if (backend == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim sch init: no backend!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    sch->m_backend = backend;
    sch->m_default_layer[0] = 0;

    TAILQ_INIT(&sch->m_runings);
    TAILQ_INIT(&sch->m_defs);
    TAILQ_INIT(&sch->m_groups);
    TAILQ_INIT(&sch->m_templates);

    if (ui_sprite_anim_group_create(sch, "") == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim sch init: create default group fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

static void ui_sprite_anim_sch_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_anim_sch_t anim_sch = ui_sprite_component_data(component);

    assert(TAILQ_EMPTY(&anim_sch->m_runings));

    while(!TAILQ_EMPTY(&anim_sch->m_templates)) {
        ui_sprite_anim_template_free(TAILQ_FIRST(&anim_sch->m_templates));
    }

    while(!TAILQ_EMPTY(&anim_sch->m_groups)) {
        ui_sprite_anim_group_t group = TAILQ_FIRST(&anim_sch->m_groups);
        assert(group->m_group_id == UI_SPRITE_INVALID_ANIM_ID);
        ui_sprite_anim_group_free(group);
    }

    while(!TAILQ_EMPTY(&anim_sch->m_defs)) {
        ui_sprite_anim_def_free(TAILQ_FIRST(&anim_sch->m_defs));
    }
}

static int ui_sprite_anim_sch_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(to);
    ui_sprite_anim_sch_t to_anim_sch = ui_sprite_component_data(to);
    ui_sprite_anim_sch_t from_anim_sch = ui_sprite_component_data(from);
    ui_sprite_anim_def_t from_anim_def;
    ui_sprite_anim_group_t from_anim_group;
    ui_sprite_anim_template_t from_anim_template;

    if (ui_sprite_anim_sch_init(to, ctx) != 0) return -1;

    TAILQ_FOREACH(from_anim_def, &from_anim_sch->m_defs, m_next_for_sch) {
        if (ui_sprite_anim_def_create(
                to_anim_sch, from_anim_def->m_anim_name, from_anim_def->m_anim_res, from_anim_def->m_auto_start)
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): anim sch copy: copy def %s ==> %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                from_anim_def->m_anim_name, from_anim_def->m_anim_res);
            ui_sprite_anim_sch_fini(to, ctx);
            return -1;
        }
    }
    
    TAILQ_FOREACH(from_anim_group, &from_anim_sch->m_groups, m_next_for_sch) {
        if (ui_sprite_anim_group_clone(to_anim_sch, from_anim_group) == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): anim sch copy: copy group %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), from_anim_group->m_name);
            ui_sprite_anim_sch_fini(to, ctx);
            return -1;
        }
    }
    
    TAILQ_FOREACH(from_anim_template, &from_anim_sch->m_templates, m_next_for_sch) {
        if (ui_sprite_anim_template_clone(to_anim_sch, from_anim_template) == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): anim sch copy: copy template %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), from_anim_template->m_name);
            ui_sprite_anim_sch_fini(to, ctx);
            return -1;
        }
    }

    return 0;
}

static void ui_sprite_anim_sch_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_anim_sch_t anim_sch = ui_sprite_component_data(component);
    ui_sprite_anim_group_t group;

    while(!TAILQ_EMPTY(&anim_sch->m_runings)) {
        ui_sprite_anim_runing_free(TAILQ_FIRST(&anim_sch->m_runings));
    }

    TAILQ_FOREACH(group, &anim_sch->m_groups, m_next_for_sch) {
        ui_sprite_anim_group_exit(group);
    }
}

static int ui_sprite_anim_sch_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_anim_sch_t anim_sch = ui_sprite_component_data(component);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_anim_def_t anim_def;
    ui_sprite_anim_group_t group;
    ui_sprite_2d_transform_t transform;

    transform = ui_sprite_2d_transform_find(entity);

    TAILQ_FOREACH(group, &anim_sch->m_groups, m_next_for_sch) {
        if (ui_sprite_anim_group_enter(group) != 0) {
            ui_sprite_anim_sch_exit(component, ctx);
            return -1;
        }

        if (transform) ui_sprite_anim_group_update(group, transform);
    }

    group = ui_sprite_anim_group_find_by_name(anim_sch, "");
    assert(group);

    TAILQ_FOREACH(anim_def, &anim_sch->m_defs, m_next_for_sch) {
        if (!anim_def->m_auto_start) continue;

        if (ui_sprite_anim_runing_create(anim_sch, group, anim_def->m_anim_res) == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): auto start anim %s(%s): start fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                anim_def->m_anim_name, anim_def->m_anim_res);
            ui_sprite_anim_sch_exit(component, ctx);
            return -1;
        }
    }

    if (ui_sprite_2d_transform_find(entity)) {
        if (ui_sprite_component_add_attr_monitor(
                component,
                "transform.pos,transform.scale,transform.angle",
                ui_sprite_anim_sch_on_transform_update,
                anim_sch)
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): add attr monitor fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_anim_sch_exit(component, ctx);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_anim_sch_set_default_layer(ui_sprite_anim_sch_t anim_sch, const char * layer_name) {
    strncpy(anim_sch->m_default_layer, layer_name, sizeof(anim_sch->m_default_layer));
}

int ui_sprite_anim_sch_regist(ui_sprite_anim_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_ANIM_SCH_NAME, sizeof(struct ui_sprite_anim_sch));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_anim_module_name(module), UI_SPRITE_ANIM_SCH_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_anim_sch_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_anim_sch_copy, module);
    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_anim_sch_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_anim_sch_exit, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_anim_sch_fini, module);

    return 0;
}

void ui_sprite_anim_sch_unregist(ui_sprite_anim_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_ANIM_SCH_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_anim_module_name(module), UI_SPRITE_ANIM_SCH_NAME);
        return;
    }

    ui_sprite_component_meta_free(meta);
}

const char * UI_SPRITE_ANIM_SCH_NAME = "Animation";
