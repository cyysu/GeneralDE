#include <assert.h>
#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_anim_camera_move_i.h"
#include "ui_sprite_anim_camera_i.h"
#include "ui_sprite_anim_camera_op_i.h"
#include "ui_sprite_anim_camera_restrict_i.h"

ui_sprite_anim_camera_move_t ui_sprite_anim_camera_move_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_CAMERA_MOVE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_camera_move_free(ui_sprite_anim_camera_move_t move) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move);
    ui_sprite_fsm_action_free(fsm_action);
}

int8_t ui_sprite_anim_camera_move_priority(ui_sprite_anim_camera_move_t move) {
    return move->m_priority;
}

void ui_sprite_anim_camera_move_set_priority(ui_sprite_anim_camera_move_t move, int8_t priority) {
    move->m_priority = priority;
}

static void ui_sprite_anim_camera_move_sync_op(
    ui_sprite_anim_camera_t camera, ui_sprite_anim_camera_move_t move, UI_SPRITE_2D_PAIR pos)
{
    UI_SPRITE_2D_PAIR target_pos;
    UI_SPRITE_ANIM_CAMERA_OP op_data;
    float target_scale;

    target_pos =
        ui_sprite_anim_camera_calc_pos_from_pos_in_screen(
            camera, pos, move->m_pos_on_screen, move->m_target_scale);
    target_scale = move->m_target_scale;

    ui_sprite_anim_camera_restrict_adj(camera, &target_pos, &target_scale);

    if (fabs(target_pos.x - camera->m_camera_pos.x) > 0.01f
        || fabs(target_pos.y - camera->m_camera_pos.y) > 0.01f
        || fabs(target_scale - camera->m_camera_scale) > 0.001f)
    {
        op_data.type = UI_SPRITE_ANIM_CAMERA_OP_TYPE_MOVE_TO_TARGET;
        op_data.data.move_to_target.target_pos.x = target_pos.x;
        op_data.data.move_to_target.target_pos.y = target_pos.y;
        op_data.data.move_to_target.target_scale = target_scale;
        op_data.data.move_to_target.max_speed = move->m_max_speed;
        op_data.data.move_to_target.duration = move->m_duration; 

        move->m_curent_op_id =
            ui_sprite_anim_camera_op_check_start(
                camera,
                move->m_curent_op_id,
                &op_data,
                move->m_priority,
                ui_sprite_anim_camera_op_suspend_suspend,
                ui_sprite_anim_camera_op_complete_keep);
    }
}

static void ui_sprite_anim_camera_move_on_follow_entity(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_move_t move = ctx;
    ui_sprite_anim_module_t module = move->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
	UI_SPRITE_EVT_ANIM_CAMERA_MOVE_FOLLOW_ENTITY const * evt_data = evt->data;
    UI_SPRITE_2D_PAIR target_pos;
    uint8_t pos_of_entity;

    pos_of_entity = ui_sprite_2d_transform_pos_policy_from_str(evt_data->pos_of_entity);
    if (pos_of_entity == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on follow to entity: pos of entity %s is unknown!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_of_entity);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    if (ui_sprite_anim_camera_pos_of_entity(&target_pos, world, evt_data->entity_id, evt_data->entity_name, pos_of_entity) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on follow to entity: get pos of entity %d(%s) fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->entity_id, evt_data->entity_name);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    move->m_follow_entity_id = evt_data->entity_id;
    strncpy(move->m_follow_entity_name, evt_data->entity_name, sizeof(move->m_follow_entity_name));
    move->m_follow_entity_pos = pos_of_entity;
    move->m_pos_on_screen.x = evt_data->pos_on_screen.x;
    move->m_pos_on_screen.y = evt_data->pos_on_screen.y;
    move->m_target_scale = evt_data->scale ? evt_data->scale : camera->m_camera_scale;;
    move->m_duration = evt_data->duration;
    move->m_max_speed = evt_data->max_speed;

    ui_sprite_anim_camera_move_sync_op(camera, move, target_pos);

    ui_sprite_fsm_action_sync_update(action, 1);
}

static void ui_sprite_anim_camera_move_on_move_to_entity(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_move_t move = ctx;
    ui_sprite_anim_module_t module = move->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
	UI_SPRITE_EVT_ANIM_CAMERA_MOVE_TO_ENTITY const * evt_data = evt->data;
    UI_SPRITE_2D_PAIR target_pos;
    uint8_t pos_of_entity;

    pos_of_entity = ui_sprite_2d_transform_pos_policy_from_str(evt_data->pos_of_entity);
    if (pos_of_entity == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on follow to entity: pos of entity %s is unknown!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_of_entity);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    if (ui_sprite_anim_camera_pos_of_entity(&target_pos, world, evt_data->entity_id, evt_data->entity_name, pos_of_entity) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on set to entity: get pos of entity %d(%s) fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->entity_id, evt_data->entity_name);
        return;
    }

    move->m_follow_entity_id = 0;
    move->m_follow_entity_name[0] = 0;
    move->m_follow_entity_pos = 0;
    move->m_pos_on_screen.x = evt_data->pos_on_screen.x;
    move->m_pos_on_screen.y = evt_data->pos_on_screen.y;
    move->m_target_scale = evt_data->scale ? evt_data->scale : camera->m_camera_scale;;
    move->m_duration = evt_data->duration;
    move->m_max_speed = evt_data->max_speed;

    ui_sprite_anim_camera_move_sync_op(camera, move, target_pos);

    ui_sprite_fsm_action_sync_update(action, 1);
}

static void ui_sprite_anim_camera_move_on_move_to_pos(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_move_t move = ctx;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
	UI_SPRITE_EVT_ANIM_CAMERA_MOVE_TO_POS const * evt_data = evt->data;
    UI_SPRITE_2D_PAIR target_pos;

    move->m_follow_entity_id = 0;
    move->m_follow_entity_name[0] = 0;
    move->m_follow_entity_pos = 0;
    move->m_pos_on_screen.x = evt_data->pos_on_screen.x;
    move->m_pos_on_screen.y = evt_data->pos_on_screen.y;
    move->m_target_scale = evt_data->scale ? evt_data->scale : camera->m_camera_scale;;
    move->m_duration = evt_data->duration;
    move->m_max_speed = evt_data->max_speed;

    target_pos.x = evt_data->pos_of_world.x;
    target_pos.y = evt_data->pos_of_world.y;
    ui_sprite_anim_camera_move_sync_op(camera, move, target_pos);

    ui_sprite_fsm_action_sync_update(action, 1);
}

static int ui_sprite_anim_camera_move_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_anim_module_t module = ctx;

	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self, 
		"ui_sprite_evt_anim_camera_move_follow_entity", ui_sprite_anim_camera_move_on_follow_entity, move) != 0)
	{    
		CPE_ERROR(module->m_em, "camera move enter: add eventer handler fail!");
		return -1;
	}
	
	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self, 
		"ui_sprite_evt_anim_camera_move_to_entity", ui_sprite_anim_camera_move_on_move_to_entity, move) != 0)
	{    
		CPE_ERROR(module->m_em, "camera move enter: add eventer handler fail!");
		return -1;
	}

	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self, 
		"ui_sprite_evt_anim_camera_move_to_pos", ui_sprite_anim_camera_move_on_move_to_pos, move) != 0)
	{    
		CPE_ERROR(module->m_em, "camera move enter: add eventer handler fail!");
		return -1;
	}
	
    return 0;
}

static void ui_sprite_anim_camera_move_do_clear(ui_sprite_anim_camera_t camera, ui_sprite_anim_camera_move_t move) {
    if (move->m_curent_op_id != UI_SPRITE_INVALID_CAMERA_OP_ID) {
        ui_sprite_anim_camera_op_stop(camera, move->m_curent_op_id);
        move->m_curent_op_id = UI_SPRITE_INVALID_CAMERA_OP_ID;
    }

    move->m_follow_entity_id = 0;
    move->m_follow_entity_name[0] = 0;
    move->m_follow_entity_pos = 0;
}

static void ui_sprite_anim_camera_move_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera move exit: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (move->m_curent_op_id != UI_SPRITE_INVALID_CAMERA_OP_ID) {
        ui_sprite_anim_camera_op_stop(camera, move->m_curent_op_id);
        move->m_curent_op_id = UI_SPRITE_INVALID_CAMERA_OP_ID;
    }
}

static void ui_sprite_anim_camera_move_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_anim_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera update: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_anim_camera_move_do_clear(camera, move);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (move->m_follow_entity_id != 0 || move->m_follow_entity_name[0] != 0) {
        UI_SPRITE_2D_PAIR new_pos;

        if (ui_sprite_anim_camera_pos_of_entity(&new_pos, world, move->m_follow_entity_id, move->m_follow_entity_name, move->m_follow_entity_pos) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera on update entity pos: get pos of entity %d(%s) fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move->m_follow_entity_id, move->m_follow_entity_name);
            ui_sprite_anim_camera_move_do_clear(camera, move);
            ui_sprite_fsm_action_stop_update(fsm_action);
            return;
        }

        ui_sprite_anim_camera_move_sync_op(camera, move, new_pos);
    }
    else {
        if (!ui_sprite_anim_camera_op_is_runing(camera, move->m_curent_op_id)) {
            ui_sprite_anim_camera_move_do_clear(camera, move);
            ui_sprite_fsm_action_stop_update(fsm_action);
        }
    }
}

static int ui_sprite_anim_camera_move_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);

    bzero(move, sizeof(*move));

    move->m_module = ctx;

    return 0;
}

static void ui_sprite_anim_camera_move_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);

    assert (move->m_curent_op_id == UI_SPRITE_INVALID_CAMERA_OP_ID);
}

static int ui_sprite_anim_camera_move_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_anim_camera_move_init(to, ctx)) return -1;
    return 0;
}

int ui_sprite_anim_camera_move_regist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_MOVE_NAME, sizeof(struct ui_sprite_anim_camera_move));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim camera move register: meta create fail",
            ui_sprite_anim_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_anim_camera_move_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_anim_camera_move_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_anim_camera_move_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_anim_camera_move_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_anim_camera_move_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_anim_camera_move_update, module);

    return 0;
}

void ui_sprite_anim_camera_move_unregist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_MOVE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim camera move unregister: meta not exist",
            ui_sprite_anim_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_ANIM_CAMERA_MOVE_NAME = "camera-move";

