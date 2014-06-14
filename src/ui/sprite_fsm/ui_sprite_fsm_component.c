#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui_sprite_fsm_component_i.h"
#include "ui_sprite_fsm_ins_i.h"
#include "ui_sprite_fsm_ins_state_i.h"
#include "ui_sprite_fsm_module_i.h"

ui_sprite_fsm_ins_t ui_sprite_fsm_component_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_FSM_COMPONENT_FSM_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

ui_sprite_fsm_ins_t ui_sprite_fsm_component_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_FSM_COMPONENT_FSM_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

static int ui_sprite_fsm_component_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_fsm_ins_t fsm = ui_sprite_component_data(component);
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);

    if (ui_sprite_fsm_ins_enter(fsm) != 0) return -1;

    if (ui_sprite_component_start_update(component) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: start update fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_fsm_ins_path(fsm));

        ui_sprite_fsm_ins_exit(fsm);

        return -1;
    }

    return 0;
}

static void ui_sprite_fsm_component_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_fsm_ins_t fsm = ui_sprite_component_data(component);

    ui_sprite_fsm_ins_exit(fsm);
}

static void ui_sprite_fsm_component_update(ui_sprite_component_t component, void * ctx, float delta) {
    ui_sprite_fsm_ins_t fsm = ui_sprite_component_data(component);

    ui_sprite_fsm_ins_update(fsm, delta);

    if (fsm->m_cur_state == NULL) {
        ui_sprite_component_stop_update(component);
        ui_sprite_entity_set_destory(ui_sprite_component_entity(component));
    }
}

static int ui_sprite_fsm_component_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_fsm_ins_init(ui_sprite_component_data(component), ctx, NULL);
    return 0;
}

static void ui_sprite_fsm_component_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_fsm_ins_fini(ui_sprite_component_data(component));
}

static int ui_sprite_fsm_component_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_fsm_ins_init(ui_sprite_component_data(to), ctx, NULL);

    return ui_sprite_fsm_ins_copy(ui_sprite_component_data(to), ui_sprite_component_data(from));
}

int ui_sprite_fsm_component_regist(ui_sprite_fsm_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(module->m_repo, UI_SPRITE_FSM_COMPONENT_FSM_NAME, sizeof(struct ui_sprite_fsm_ins));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: fsm component register: meta create fail",
            ui_sprite_fsm_module_name(module));
        return -1;
    }

    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_fsm_component_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_fsm_component_exit, module);
    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_fsm_component_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_fsm_component_copy, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_fsm_component_fini, module);
    ui_sprite_component_meta_set_update_fun(meta, ui_sprite_fsm_component_update, module);

    return 0;
}

void ui_sprite_fsm_component_unregist(ui_sprite_fsm_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_FSM_COMPONENT_FSM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: fsm component unregister: meta not exist",
            ui_sprite_fsm_module_name(module));
        return;
    }

    ui_sprite_component_meta_free(meta);
}

/*proto*/
ui_sprite_fsm_ins_t ui_sprite_fsm_proto_create(ui_sprite_world_t world, const char * name) {
    ui_sprite_entity_t proto_entity;
    ui_sprite_fsm_ins_t proto_fsm;

    proto_entity = ui_sprite_entity_proto_create(world, name);
    if (proto_entity == NULL) return NULL;

    proto_fsm = ui_sprite_fsm_component_create(proto_entity);
    if (proto_fsm == NULL) {
        ui_sprite_entity_free(proto_entity);
        return NULL;
    }

    return proto_fsm;
}

void ui_sprite_fsm_proto_free(ui_sprite_fsm_ins_t proto_fsm) {
    ui_sprite_entity_t proto_entity;

    proto_entity = ui_sprite_fsm_to_entity(proto_fsm);
    ui_sprite_entity_free(proto_entity);
}

ui_sprite_fsm_ins_t ui_sprite_fsm_proto_find(ui_sprite_world_t world, const char * name) {
    ui_sprite_entity_t proto_entity;

    proto_entity = ui_sprite_entity_proto_find(world, name);

    return proto_entity == NULL
        ? NULL
        : ui_sprite_fsm_component_find(proto_entity);
}

const char * UI_SPRITE_FSM_COMPONENT_FSM_NAME = "Fsm";
