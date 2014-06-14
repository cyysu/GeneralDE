#include <assert.h>
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui_sprite_fsm_ins_i.h"
#include "ui_sprite_fsm_ins_state_i.h"
#include "ui_sprite_fsm_ins_action_i.h"
#include "ui_sprite_fsm_action_meta_i.h"
#include "ui_sprite_fsm_action_fsm_i.h"
#include "protocol/ui/sprite_fsm/ui_sprite_fsm_evt.h"

ui_sprite_fsm_ins_t ui_sprite_fsm_action_fsm_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_FSM_ACTION_FSM_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

static void ui_sprite_fsm_action_fsm_on_run_fsm(void * ctx, ui_sprite_event_t evt) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ctx;
    ui_sprite_fsm_ins_t fsm = &action_fsm->m_ins;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(fsm);
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_fsm_ins_t proto_fsm;

    UI_SPRITE_EVT_FSM_RUN_FSM const * evt_data = evt->data;
    
    if (strcmp(fsm_action->m_name, evt_data->action) != 0) return;

    proto_fsm = ui_sprite_fsm_proto_find(ui_sprite_entity_world(entity), evt_data->load_from);
    if (proto_fsm == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s) load fsm from %s: proto entity no fsm component",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name, evt_data->load_from);
        return;
    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): %s: action %s(%s) load fsm from %s: clear old fsm!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name, evt_data->load_from);
    }

    ui_sprite_fsm_ins_reinit(fsm);
    if (ui_sprite_fsm_ins_copy(fsm, proto_fsm) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s) load fsm from %s: copy fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name, evt_data->load_from);
        ui_sprite_fsm_ins_reinit(fsm);
        action_fsm->m_auto_clear = 0;
        return;
    }

    if (ui_sprite_fsm_ins_enter(fsm) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s) load fsm from %s: enter fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name, evt_data->load_from);
        ui_sprite_fsm_ins_reinit(fsm);
        action_fsm->m_auto_clear = 0;
        return;
    }

    if (ui_sprite_fsm_action_start_update(fsm_action) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s) load fsm from %s: start update fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name, evt_data->load_from);
        ui_sprite_fsm_ins_reinit(fsm);
        action_fsm->m_auto_clear = 0;
        return;
    }

    action_fsm->m_auto_clear = 1;
}

static int ui_sprite_fsm_action_fsm_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_fsm_ins_t fsm = &action_fsm->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self, 
		"ui_sprite_evt_fsm_run_fsm", ui_sprite_fsm_action_fsm_on_run_fsm, fsm) != 0)
	{    
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: action %s(%s) add event handler fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            fsm_action->m_meta->m_name, fsm_action->m_name);
        return -1;
	}

    if (!TAILQ_EMPTY(&fsm->m_states)) {
        if (ui_sprite_fsm_ins_enter(fsm) != 0) return -1;
        if (ui_sprite_fsm_action_start_update(fsm_action) != 0) return -1;
    }

    return 0;
}

static void ui_sprite_fsm_action_fsm_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_fsm_ins_t fsm = &action_fsm->m_ins;

    ui_sprite_fsm_ins_exit(fsm);
}

static void ui_sprite_fsm_action_fsm_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_fsm_module_t module = ctx;
    ui_sprite_fsm_ins_t fsm = &action_fsm->m_ins;

    assert(fsm_action->m_runing_state == ui_sprite_fsm_action_state_runing);

    ui_sprite_fsm_ins_update(fsm, delta);

    if (fsm->m_cur_state == NULL) {
        ui_sprite_fsm_action_stop_update(fsm_action);

        if (action_fsm->m_auto_clear) {
            ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): %s: action %s(%s): auto clear!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                    fsm_action->m_meta->m_name, fsm_action->m_name);
            }

            ui_sprite_fsm_ins_reinit(&action_fsm->m_ins);
            action_fsm->m_auto_clear = 0;
        }
    }
}

static int ui_sprite_fsm_action_fsm_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ui_sprite_fsm_action_data(fsm_action);

    assert(fsm_action->m_state);

    ui_sprite_fsm_ins_init(&action_fsm->m_ins, ctx, fsm_action->m_state->m_ins);
    action_fsm->m_auto_clear = 0;

    return 0;
}

static void ui_sprite_fsm_action_fsm_fini(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    struct ui_sprite_fsm_action_fsm * action_fsm = ui_sprite_fsm_action_data(fsm_action);

    ui_sprite_fsm_ins_fini(&action_fsm->m_ins);
}

static int ui_sprite_fsm_action_fsm_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    struct ui_sprite_fsm_action_fsm * to_action_fsm = ui_sprite_fsm_action_data(to);
    struct ui_sprite_fsm_action_fsm * from_action_fsm = ui_sprite_fsm_action_data(from);

    ui_sprite_fsm_action_fsm_init(to, ctx);

    if (ui_sprite_fsm_ins_copy(&to_action_fsm->m_ins, &from_action_fsm->m_ins) != 0) {
        ui_sprite_fsm_action_fsm_fini(to, ctx);
        return -1;
    }

    to_action_fsm->m_auto_clear = 0;

    return 0;
}

int ui_sprite_fsm_action_fsm_regist(ui_sprite_fsm_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module, UI_SPRITE_FSM_ACTION_FSM_NAME, sizeof(struct ui_sprite_fsm_action_fsm));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: fsm fsm_action register: meta create fail",
            ui_sprite_fsm_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_fsm_action_fsm_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_fsm_action_fsm_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_fsm_action_fsm_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_fsm_action_fsm_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_fsm_action_fsm_fini, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_fsm_action_fsm_update, module);

    return 0;
}

void ui_sprite_fsm_action_fsm_unregist(ui_sprite_fsm_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module, UI_SPRITE_FSM_ACTION_FSM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: fsm fsm_action unregister: meta not exist",
            ui_sprite_fsm_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_FSM_ACTION_FSM_NAME = "run-fsm";
