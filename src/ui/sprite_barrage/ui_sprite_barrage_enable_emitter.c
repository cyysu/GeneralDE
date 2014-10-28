#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_barrage_enable_emitter_i.h"
#include "ui_sprite_barrage_obj_i.h"
#include "ui_sprite_barrage_emitter_i.h"

ui_sprite_barrage_enable_emitter_t ui_sprite_barrage_enable_emitter_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BARRAGE_ENABLE_EMITTER_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_barrage_enable_emitter_free(ui_sprite_barrage_enable_emitter_t enable_emitter) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(enable_emitter);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_barrage_enable_emitter_set_group_name(ui_sprite_barrage_enable_emitter_t enable_emitter, const char * name) {
    
    ui_sprite_barrage_module_t module = enable_emitter->m_module;

    if (enable_emitter->m_group_name) mem_free(module->m_alloc, enable_emitter->m_group_name);

    if (name) {
        enable_emitter->m_group_name = cpe_str_mem_dup(module->m_alloc, name);
        return enable_emitter->m_group_name == NULL ? -1 : 0;
    }
    else {
        enable_emitter->m_group_name = NULL;
        return 0;
    }
}

const char * ui_sprite_barrage_enable_emitter_group_name(ui_sprite_barrage_enable_emitter_t enable_emitter) {
    return enable_emitter->m_group_name;
}

static int ui_sprite_barrage_enable_emitter_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_emitter_t enable_emitter = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj;

    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage enable emitter: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_barrage_obj_set_emitters_is_enable(barrage_obj, enable_emitter->m_group_name, 1);

    return 0;
}

static void ui_sprite_barrage_enable_emitter_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_emitter_t enable_emitter = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj;

    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage enable emitter: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_sprite_barrage_obj_set_emitters_is_enable(barrage_obj, enable_emitter->m_group_name, 0);
}

static int ui_sprite_barrage_enable_emitter_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_enable_emitter_t enable_emitter = ui_sprite_fsm_action_data(fsm_action);
    enable_emitter->m_module = ctx;
	enable_emitter->m_group_name = NULL;
    return 0;
}

static void ui_sprite_barrage_enable_emitter_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_emitter_t enable_emitter = ui_sprite_fsm_action_data(fsm_action);

    if (enable_emitter->m_group_name) {
        mem_free(module->m_alloc, enable_emitter->m_group_name);
        enable_emitter->m_group_name = NULL;
    }
}

static int ui_sprite_barrage_enable_emitter_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_emitter_t to_enable_emitter = ui_sprite_fsm_action_data(to);
    ui_sprite_barrage_enable_emitter_t from_enable_emitter = ui_sprite_fsm_action_data(from);

    if (ui_sprite_barrage_enable_emitter_init(to, ctx)) return -1;

    if (from_enable_emitter->m_group_name) {
        to_enable_emitter->m_group_name = cpe_str_mem_dup(module->m_alloc, from_enable_emitter->m_group_name);
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_barrage_enable_emitter_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_emitter_t enable_emitter = ui_sprite_barrage_enable_emitter_create(fsm_state, name);

    if (enable_emitter == NULL) {
        CPE_ERROR(module->m_em, "%s: create enable_emitter action: create fail!", ui_sprite_barrage_module_name(module));
        return NULL;
    }

    if (ui_sprite_barrage_enable_emitter_set_group_name(enable_emitter, cfg_get_string(cfg, "group-name", NULL)) != 0) {
        CPE_ERROR(module->m_em, "%s: create enable_emitter action: create fail!", ui_sprite_barrage_module_name(module));
        ui_sprite_barrage_enable_emitter_free(enable_emitter);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(enable_emitter);
}

int ui_sprite_barrage_enable_emitter_regist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_BARRAGE_ENABLE_EMITTER_NAME, sizeof(struct ui_sprite_barrage_enable_emitter));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: barrage enable emitter register: meta create fail",
            ui_sprite_barrage_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_barrage_enable_emitter_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_barrage_enable_emitter_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_barrage_enable_emitter_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_barrage_enable_emitter_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_barrage_enable_emitter_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_BARRAGE_ENABLE_EMITTER_NAME, ui_sprite_barrage_enable_emitter_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_barrage_enable_emitter_unregist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BARRAGE_ENABLE_EMITTER_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_barrage_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BARRAGE_ENABLE_EMITTER_NAME);
}

const char * UI_SPRITE_BARRAGE_ENABLE_EMITTER_NAME = "enable-barrage-emitter";

