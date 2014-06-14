#include <math.h>
#include <assert.h>
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_op.h"
#include "ui_sprite_anim_camera_touch_i.h"
#include "ui_sprite_anim_camera_i.h"
#include "ui_sprite_anim_camera_op_i.h"

ui_sprite_anim_camera_touch_t ui_sprite_anim_camera_touch_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_CAMERA_TOUCH_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_camera_touch_free(ui_sprite_anim_camera_touch_t touch) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(touch);
    ui_sprite_fsm_action_free(fsm_action);
}

int8_t ui_sprite_anim_camera_touch_priority(ui_sprite_anim_camera_touch_t touch) {
    return touch->m_priority;
}

void ui_sprite_anim_camera_touch_set_priority(ui_sprite_anim_camera_touch_t touch, int8_t priority) {
    touch->m_priority = priority;
}

static void ui_sprite_anim_camera_touch_sync_update(ui_sprite_anim_camera_t camera, ui_sprite_anim_camera_touch_t touch) {
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(touch);
    ui_sprite_anim_camera_op_t op;

    if (touch->m_curent_op_id == UI_SPRITE_INVALID_CAMERA_OP_ID) {
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    op = ui_sprite_anim_camera_op_find(camera, touch->m_curent_op_id);
    if (op == NULL) {
        touch->m_curent_op_id = UI_SPRITE_INVALID_CAMERA_OP_ID;
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    if (op->m_is_done) {
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    ui_sprite_fsm_action_sync_update(action, 1);
}

static void ui_sprite_anim_camera_touch_on_begin(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_touch_t touch = ctx;
    ui_sprite_anim_module_t module = touch->m_module;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
    UI_SPRITE_ANIM_CAMERA_OP op_data;

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-begin: world no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    assert(touch->m_curent_op_id == UI_SPRITE_INVALID_CAMERA_OP_ID);
    if (touch->m_curent_op_id != UI_SPRITE_INVALID_CAMERA_OP_ID) {
        ui_sprite_anim_camera_op_stop(camera, touch->m_curent_op_id);
        touch->m_curent_op_id = UI_SPRITE_INVALID_CAMERA_OP_ID;
    }

    op_data.type = UI_SPRITE_ANIM_CAMERA_OP_TYPE_MOVE_TO_TARGET;

    op_data.data.move_to_target.target_pos.x = camera->m_camera_pos.x;
    op_data.data.move_to_target.target_pos.y = camera->m_camera_pos.y;
    op_data.data.move_to_target.target_scale = camera->m_camera_scale;
    op_data.data.move_to_target.max_speed = 0.0f;
    op_data.data.move_to_target.duration = 0.0f;

    touch->m_curent_op_id =
        ui_sprite_anim_camera_op_check_start(
            camera, touch->m_curent_op_id, &op_data,
            touch->m_priority,
            ui_sprite_anim_camera_op_suspend_suspend,
            ui_sprite_anim_camera_op_complete_keep);

    ui_sprite_anim_camera_touch_sync_update(camera, touch);
}

static void ui_sprite_anim_camera_touch_on_move(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_touch_t touch = ctx;
    ui_sprite_anim_module_t module = touch->m_module;
    UI_SPRITE_EVT_ANIM_CAMERA_TOUCH_MOVE const * evt_data = evt->data;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
    UI_SPRITE_ANIM_CAMERA_OP op_data;

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-move: world no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    op_data.type = UI_SPRITE_ANIM_CAMERA_OP_TYPE_MOVE_TO_TARGET;

    op_data.data.move_to_target.target_pos.x =
        camera->m_camera_pos.x
        + (evt_data->old_screen_pos.x - evt_data->new_screen_pos.x) / camera->m_camera_scale_pair.x;

    op_data.data.move_to_target.target_pos.y =
        camera->m_camera_pos.y
        + (evt_data->old_screen_pos.y - evt_data->new_screen_pos.y) / camera->m_camera_scale_pair.y;

    op_data.data.move_to_target.target_scale = camera->m_camera_scale;

    op_data.data.move_to_target.max_speed = evt_data->max_speed;
    op_data.data.move_to_target.duration = evt_data->duration; 

    touch->m_curent_op_id =
        ui_sprite_anim_camera_op_check_start(
            camera,
            touch->m_curent_op_id,
            &op_data,
            touch->m_priority,
            ui_sprite_anim_camera_op_suspend_suspend,
            ui_sprite_anim_camera_op_complete_keep);

    ui_sprite_anim_camera_touch_sync_update(camera, touch);
}

static void ui_sprite_anim_camera_touch_on_end(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_touch_t touch = ctx;
    ui_sprite_anim_module_t module = touch->m_module;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
    UI_SPRITE_EVT_ANIM_CAMERA_TOUCH_END const * evt_data = evt->data;
	float rab = 0.0f;
    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-end: world no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (touch->m_curent_op_id != UI_SPRITE_INVALID_CAMERA_OP_ID) {
        ui_sprite_anim_camera_op_stop(camera, touch->m_curent_op_id);
        touch->m_curent_op_id = UI_SPRITE_INVALID_CAMERA_OP_ID;
    }

    if (fabs(evt_data->speed.x) > 0.0f || fabs(evt_data->speed.y) > 0.0f) {
        UI_SPRITE_ANIM_CAMERA_OP op_data;

        if (evt_data->reduce_per_step <= 0.0f) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera-touch: on-end: reduce_per_step error!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return;
        }

        if (evt_data->step <= 0.0f) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera-touch: on-end: step error!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return;
        }

        op_data.type = UI_SPRITE_ANIM_CAMERA_OP_TYPE_MOVE_BY_SPEED;
        op_data.data.move_by_speed.speed = evt_data->speed;
		rab = atan2f(fabs(evt_data->speed.y), fabs(evt_data->speed.x)); 
        op_data.data.move_by_speed.reduce_per_step.x = cos(rab) * evt_data->reduce_per_step;
        op_data.data.move_by_speed.reduce_per_step.y = sin(rab) * evt_data->reduce_per_step;
        op_data.data.move_by_speed.step = evt_data->step;
        op_data.data.move_by_speed.left_time = 0.0f;
        touch->m_curent_op_id =
            ui_sprite_anim_camera_op_start(
                camera, &op_data,
                touch->m_priority,
                ui_sprite_anim_camera_op_suspend_remove,
                ui_sprite_anim_camera_op_complete_remove);
    }

    ui_sprite_anim_camera_touch_sync_update(camera, touch);
}

static int ui_sprite_anim_camera_touch_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_touch_t touch = ui_sprite_fsm_action_data(fsm_action); 
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if ((ui_sprite_fsm_action_add_event_handler(
             fsm_action, ui_sprite_event_scope_self, 
             "ui_sprite_evt_anim_camera_touch_begin",
             ui_sprite_anim_camera_touch_on_begin, touch) != 0)
        ||
        (ui_sprite_fsm_action_add_event_handler(
             fsm_action, ui_sprite_event_scope_self, 
             "ui_sprite_evt_anim_camera_touch_move",
             ui_sprite_anim_camera_touch_on_move, touch) != 0)
        ||
        (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_anim_camera_touch_end",
            ui_sprite_anim_camera_touch_on_end, touch) != 0)
        )
        
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-run-op: enter: add eventer handler fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

static void ui_sprite_anim_camera_touch_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_touch_t touch = ui_sprite_fsm_action_data(fsm_action); 

    if (touch->m_curent_op_id != UI_SPRITE_INVALID_CAMERA_OP_ID) {
        ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
        ui_sprite_world_t world = ui_sprite_entity_world(entity);  
        ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
        ui_sprite_anim_module_t module = ctx;

        if (camera == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera-run-op: world no camera!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return;
        }

        ui_sprite_anim_camera_op_stop(camera, touch->m_curent_op_id);
        touch->m_curent_op_id = UI_SPRITE_INVALID_CAMERA_OP_ID;
    }
}

static int ui_sprite_anim_camera_touch_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_touch_t touch = ui_sprite_fsm_action_data(fsm_action);

    touch->m_module = ctx;
    touch->m_curent_op_id = UI_SPRITE_INVALID_CAMERA_OP_ID;
    touch->m_priority = 0;

    return 0;
}

static void ui_sprite_anim_camera_touch_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_touch_t touch = ui_sprite_fsm_action_data(fsm_action);

    assert(touch->m_curent_op_id == UI_SPRITE_INVALID_CAMERA_OP_ID);
}

static int ui_sprite_anim_camera_touch_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_anim_camera_touch_t touch_to = ui_sprite_fsm_action_data(to);
    ui_sprite_anim_camera_touch_t touch_from = ui_sprite_fsm_action_data(from);

    if (ui_sprite_anim_camera_touch_init(to, ctx)) return -1;

    touch_to->m_priority = touch_from->m_priority;

    return 0;
}

static void ui_sprite_anim_camera_touch_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_anim_camera_touch_t touch = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);  
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
    ui_sprite_anim_module_t module = ctx;

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-run-op: update: world no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (touch->m_curent_op_id == UI_SPRITE_INVALID_CAMERA_OP_ID
        || !ui_sprite_anim_camera_op_is_runing(camera, touch->m_curent_op_id))
    {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

int ui_sprite_anim_camera_touch_regist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_TOUCH_NAME, sizeof(struct ui_sprite_anim_camera_touch));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim camera touch register: meta create fail",
            ui_sprite_anim_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_anim_camera_touch_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_anim_camera_touch_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_anim_camera_touch_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_anim_camera_touch_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_anim_camera_touch_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_anim_camera_touch_update, module);

    return 0;
}

void ui_sprite_anim_camera_touch_unregist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_TOUCH_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim camera touch unregister: meta not exist",
            ui_sprite_anim_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_ANIM_CAMERA_TOUCH_NAME = "camera-touch";

