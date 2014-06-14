#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_anim_camera_wait_stop_i.h"
#include "ui_sprite_anim_camera_op_i.h"

ui_sprite_anim_camera_wait_stop_t ui_sprite_anim_camera_wait_stop_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_CAMERA_WAIT_STOP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_camera_wait_stop_free(ui_sprite_anim_camera_wait_stop_t wait_stop) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(wait_stop);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_anim_camera_wait_stop_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_fsm_action_start_update(fsm_action);
    return 0;
}

static void ui_sprite_anim_camera_wait_stop_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_anim_camera_wait_stop_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_wait_stop_t wait_stop = ui_sprite_fsm_action_data(fsm_action);
    wait_stop->m_module = ctx;
    return 0;
}

static void ui_sprite_anim_camera_wait_stop_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_anim_camera_wait_stop_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_anim_camera_wait_stop_init(to, ctx)) return -1;
    return 0;
}

static void ui_sprite_anim_camera_wait_stop_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
    ui_sprite_anim_camera_op_t curent_op;

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-wait-stop: update: world no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    curent_op = TAILQ_FIRST(&camera->m_ops);

    if (curent_op == NULL || curent_op->m_is_done) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

int ui_sprite_anim_camera_wait_stop_regist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_WAIT_STOP_NAME, sizeof(struct ui_sprite_anim_camera_wait_stop));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim camera wait_stop register: meta create fail",
            ui_sprite_anim_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_anim_camera_wait_stop_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_anim_camera_wait_stop_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_anim_camera_wait_stop_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_anim_camera_wait_stop_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_anim_camera_wait_stop_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_anim_camera_wait_stop_update, module);

    return 0;
}

void ui_sprite_anim_camera_wait_stop_unregist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_WAIT_STOP_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim camera wait_stop unregister: meta not exist",
            ui_sprite_anim_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_ANIM_CAMERA_WAIT_STOP_NAME = "camera-wait-stop";

