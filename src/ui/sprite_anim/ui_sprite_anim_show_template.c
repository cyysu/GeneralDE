#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/xcalc/xcalc_token.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_anim/ui_sprite_anim_sch.h"
#include "ui_sprite_anim_show_template_i.h"
#include "ui_sprite_anim_module_i.h"
#include "ui_sprite_anim_backend_i.h"
#include "ui_sprite_anim_template_i.h"
#include "ui_sprite_anim_template_binding_i.h"

ui_sprite_anim_show_template_t
ui_sprite_anim_show_template_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_SHOW_TEMPLATE_TYPE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_show_template_free(ui_sprite_anim_show_template_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

const char * ui_sprite_anim_show_template_group(ui_sprite_anim_show_template_t show_template) {
    return show_template->m_group;
}

void ui_sprite_anim_show_template_set_group(ui_sprite_anim_show_template_t show_template, const char * group) {
    strncpy(show_template->m_group, group, sizeof(show_template->m_group));
}


void ui_sprite_anim_show_template_exit(ui_sprite_fsm_action_t fsm_action, void * ctx);

struct ui_sprite_anim_show_collect_binding_ctx {
    ui_sprite_anim_show_template_t m_show_template;
    ui_sprite_anim_template_binding_t m_binding;
    ui_sprite_entity_t m_entity;
};

static void ui_sprite_anim_show_collect_binding(void * input_ctx, xtoken_t arg) {
    struct ui_sprite_anim_show_collect_binding_ctx * ctx = input_ctx;
    ui_sprite_anim_module_t module = ctx->m_show_template->m_module;
    ui_sprite_anim_show_template_binding_t show_binding;
    const char * arg_name = xtoken_try_to_str(arg);

    if (arg_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim_show_template: collect binding, get arg name fail",
            ui_sprite_entity_id(ctx->m_entity), ui_sprite_entity_name(ctx->m_entity));
        return;;
    }

    if (arg_name[0] == '[') return;

    TAILQ_FOREACH(show_binding, &ctx->m_show_template->m_bindings, m_next) {
        if (show_binding->m_binding == ctx->m_binding && strcmp(show_binding->m_attr_name, arg_name) == 0) return;
    }

    if (ui_sprite_anim_show_template_binding_create(ctx->m_show_template, ctx->m_binding, arg_name) == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim_show_template: collect binding, crate binding %s fail",
            ui_sprite_entity_id(ctx->m_entity), ui_sprite_entity_name(ctx->m_entity), arg_name);
        return ;
    }
}

int ui_sprite_anim_show_template_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_anim_show_template_t show_template = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    xcomputer_t computer = ui_sprite_world_computer(ui_sprite_entity_world(entity));
    ui_sprite_anim_sch_t anim_sch = ui_sprite_anim_sch_find(entity);
    ui_sprite_anim_template_t tpl;
    ui_sprite_anim_template_binding_t binding;

    if (anim_sch == NULL) {
        CPE_ERROR(
            show_template->m_module->m_em, "entity %d(%s): anim_show_template: no anim_sch, can`t start anim %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_template);
        return -1;
    }

    if (show_template->m_anim_id != UI_SPRITE_INVALID_ANIM_ID) {
        CPE_ERROR(
            show_template->m_module->m_em, "entity %d(%s): anim_show_template: show template enter: already started!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    tpl = ui_sprite_anim_template_find(anim_sch, show_template->m_template);
    if (tpl == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim_show_template: start template %s: template not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_template);
        return -1;
    }

    show_template->m_anim_id = ui_sprite_anim_sch_start_anim(anim_sch, show_template->m_group, tpl->m_res, 1, 0, 0);
    if (show_template->m_anim_id == UI_SPRITE_INVALID_ANIM_ID) {
        CPE_ERROR(
            show_template->m_module->m_em, "entity %d(%s): anim_show_template: start template %s: start anim fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_template);
        return -1;
    }

    TAILQ_FOREACH(binding, &tpl->m_bindings, m_next_for_template) {
        struct ui_sprite_anim_show_collect_binding_ctx collect_ctx = { show_template, binding, entity };

        if (ui_sprite_anim_template_binding_set_value(binding, tpl, show_template->m_anim_id, entity) != 0) {
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    show_template->m_module->m_em, "entity %d(%s): anim_show_template: start template %s: set attr %s.%s fail",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_template,
                    binding->m_control, binding->m_attr_name);
            }
        }

        if (xcomputer_visit_args(computer, binding->m_attr_value, &collect_ctx, ui_sprite_anim_show_collect_binding) != 0) {
            ui_sprite_anim_show_template_exit(fsm_action, ctx);
            return -1;
        }
    }

    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    return 0;
}

void ui_sprite_anim_show_template_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_show_template_t show_template = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_anim_sch_t anim_sch = ui_sprite_anim_sch_find(entity);

    if (anim_sch == NULL) {
        CPE_ERROR(
            show_template->m_module->m_em, "entity %d(%s): anim_show_template: no anim_sch, can`t stop anim %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_template);
        return;
    }

    while(!TAILQ_EMPTY(&show_template->m_bindings)) {
        ui_sprite_anim_show_template_binding_free(TAILQ_FIRST(&show_template->m_bindings));
    }

    if (show_template->m_anim_id != UI_SPRITE_INVALID_ANIM_ID) {
        ui_sprite_anim_sch_stop_anim(anim_sch, show_template->m_anim_id);
        show_template->m_anim_id = UI_SPRITE_INVALID_ANIM_ID;
    }
}

void ui_sprite_anim_show_template_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_anim_show_template_t show_template = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_anim_sch_t anim_sch = ui_sprite_anim_sch_find(entity);

    if (anim_sch == NULL) {
        CPE_ERROR(
            show_template->m_module->m_em, "entity %d(%s): anim_show_template: no anim_sch, template %s stop update!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_template);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (!ui_sprite_anim_sch_is_anim_runing(anim_sch, show_template->m_anim_id)) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                show_template->m_module->m_em, "entity %d(%s): anim_show_template: template %s stoped",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_template);
        }
        
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
}

int ui_sprite_anim_show_template_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_show_template_t show_template = ui_sprite_fsm_action_data(fsm_action);

    bzero(show_template, sizeof(*show_template));

    show_template->m_module = ctx;
    show_template->m_anim_id = UI_SPRITE_INVALID_ANIM_ID;
    TAILQ_INIT(&show_template->m_bindings);

    return 0;
}

int ui_sprite_anim_show_template_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_anim_show_template_t to_show_template = ui_sprite_fsm_action_data(to);
    ui_sprite_anim_show_template_t from_show_template = ui_sprite_fsm_action_data(from);

    ui_sprite_anim_show_template_init(to, ctx);

    strncpy(to_show_template->m_template, from_show_template->m_template, sizeof(to_show_template->m_template));

    return 0;
}

void ui_sprite_anim_show_template_fini(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_show_template_t show_template = ui_sprite_fsm_action_data(fsm_action);

    assert(TAILQ_EMPTY(&show_template->m_bindings));
}

int ui_sprite_anim_show_template_regist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_ANIM_SHOW_TEMPLATE_TYPE_NAME, sizeof(struct ui_sprite_anim_show_template));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_anim_module_name(module), UI_SPRITE_ANIM_SHOW_TEMPLATE_TYPE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_anim_show_template_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_anim_show_template_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_anim_show_template_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_anim_show_template_exit, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_anim_show_template_update, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_anim_show_template_fini, module);

    return 0;
}

void ui_sprite_anim_show_template_unregist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_ANIM_SHOW_TEMPLATE_TYPE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_anim_module_name(module), UI_SPRITE_ANIM_SHOW_TEMPLATE_TYPE_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * ui_sprite_anim_show_template_template(ui_sprite_anim_show_template_t show_template) {
    return show_template->m_template;
}

void ui_sprite_anim_show_template_set_template(ui_sprite_anim_show_template_t show_template, const char * res) {
    strncpy(show_template->m_template, res, sizeof(show_template->m_template));
}


void ui_sprite_anim_show_template_binding_on_attr_updated(void * ctx) {
    ui_sprite_anim_show_template_binding_t binding = ctx;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(binding->m_show_template);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_anim_sch_t anim_sch = ui_sprite_anim_sch_find(entity);
    ui_sprite_anim_module_t module = binding->m_show_template->m_module;
    ui_sprite_anim_template_t tpl;

    if (anim_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim_show_template: template %s: no anim sch",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), binding->m_show_template->m_template);
        return;
    }

    tpl = ui_sprite_anim_template_find(anim_sch, binding->m_show_template->m_template);
    if (tpl == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim_show_template: template %s: template not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), binding->m_show_template->m_template);
        return;
    }

    if (ui_sprite_anim_template_binding_set_value(binding->m_binding, tpl, binding->m_show_template->m_anim_id, entity) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim_show_template: template %s: set attr %s.%s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), binding->m_show_template->m_template,
            binding->m_binding->m_control, binding->m_binding->m_attr_name);
    }
}

ui_sprite_anim_show_template_binding_t
ui_sprite_anim_show_template_binding_create(
    ui_sprite_anim_show_template_t show_template, ui_sprite_anim_template_binding_t binding, const char * attr_name) {
    ui_sprite_anim_module_t module = show_template->m_module;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(show_template);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_anim_show_template_binding_t show_template_binding;
    size_t attr_name_len = strlen(attr_name) + 1;

    show_template_binding = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_anim_show_template_binding) + attr_name_len);
    if (show_template_binding == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim_show_template: start template %s: create binding %s.%s: alloc fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_template,
            binding->m_control, binding->m_attr_name);
        return NULL;
    }

    if (ui_sprite_fsm_action_add_attr_monitor(
            fsm_action,
            attr_name, ui_sprite_anim_show_template_binding_on_attr_updated, show_template_binding)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim_show_template: start template %s: create binding %s.%s: add monitor fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_template,
            binding->m_control, attr_name);
        mem_free(module->m_alloc, show_template_binding);
        return NULL;
    }

    show_template_binding->m_show_template = show_template;
    show_template_binding->m_binding = binding;
    show_template_binding->m_attr_name = (char*)(show_template_binding + 1);
    memcpy(show_template_binding + 1, attr_name, attr_name_len);

    TAILQ_INSERT_TAIL(&show_template->m_bindings, show_template_binding, m_next);

    return show_template_binding;
}

void ui_sprite_anim_show_template_binding_free(ui_sprite_anim_show_template_binding_t binding) {
    ui_sprite_anim_show_template_t show_template = binding->m_show_template;
    ui_sprite_anim_module_t module = show_template->m_module;

    TAILQ_REMOVE(&show_template->m_bindings, binding, m_next);

    mem_free(module->m_alloc, binding);
}


const char * UI_SPRITE_ANIM_SHOW_TEMPLATE_TYPE_NAME = "show-template";
