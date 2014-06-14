#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_move_i.h"
#include "ui_sprite_2d_module_i.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_evt.h"

ui_sprite_2d_move_t
ui_sprite_2d_move_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_MOVE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_move_free(ui_sprite_2d_move_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

static void ui_sprite_2d_move_on_follow_entity(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_2d_move_t move = ctx;
    ui_sprite_2d_module_t module = move->m_module;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    UI_SPRITE_EVT_2D_MOVE_FOLLOW_ENTITY const * evt_data = evt->data;

    move->m_follow_entity_id = evt_data->entity_id;
    strncpy(move->m_follow_entity_name, evt_data->entity_name, sizeof(move->m_follow_entity_name));
    if (move->m_follow_entity_id == 0 && evt_data->entity_name[0] == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): 2d move: follow entity: entity id and name not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }

    move->m_follow_entity_pos = ui_sprite_2d_transform_pos_policy_from_str(evt_data->pos_of_entity);
    if (move->m_follow_entity_pos == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): 2d move: follow entity: pos of entity %s is unknown!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_of_entity);
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }

    move->m_speed = evt_data->speed;
    move->m_duration = evt_data->duration;

    ui_sprite_fsm_action_sync_update(ui_sprite_fsm_action_from_data(move), 1);
}

static void ui_sprite_2d_move_on_move_to(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_2d_move_t move = ctx;
    UI_SPRITE_EVT_2D_MOVE_TO const * evt_data = evt->data;

    move->m_follow_entity_id = 0;
    move->m_follow_entity_name[0] = 0;
    move->m_follow_entity_pos = 0;
    move->m_speed = evt_data->speed;
    move->m_duration = evt_data->duration;
    move->m_target_pos = evt_data->pos;

    ui_sprite_fsm_action_sync_update(ui_sprite_fsm_action_from_data(move), 1);
}

static void ui_sprite_2d_move_on_set_to_entity(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_2d_move_t move = ctx;
    ui_sprite_2d_module_t module = move->m_module;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    ui_sprite_entity_t set_to_entity;
    ui_sprite_2d_transform_t set_to_transform;
    uint8_t pos_of_entity;

    UI_SPRITE_EVT_2D_MOVE_SET_TO_ENTITY const * evt_data = evt->data;

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move on set to entity: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }

    if (evt_data->entity_id > 0) {
        set_to_entity = ui_sprite_entity_find_by_id(ui_sprite_entity_world(entity), evt_data->entity_id);
        if (set_to_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): move on set to entity: entity %d not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->entity_id);
            ui_sprite_fsm_action_sync_update(fsm_action, 0);
            return;
        }
    }
    else if (evt_data->entity_name[0]) {
        set_to_entity = ui_sprite_entity_find_by_name(ui_sprite_entity_world(entity), evt_data->entity_name);
        if (set_to_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): move on set to entity: entity %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->entity_name);
            ui_sprite_fsm_action_sync_update(fsm_action, 0);
            return;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move on set to entity: entity not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }

    set_to_transform = ui_sprite_2d_transform_find(set_to_entity);
    if (set_to_transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move on set to entity: entity %d(%s) no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_entity_id(set_to_entity), ui_sprite_entity_name(set_to_entity));
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }

    pos_of_entity = ui_sprite_2d_transform_pos_policy_from_str(evt_data->pos_of_entity);
    if (pos_of_entity == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move on set to entity: pos of entity %s is unknown!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_of_entity);
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }

    ui_sprite_2d_transform_set_pos(
        transform,
        ui_sprite_2d_transform_pos(set_to_transform, pos_of_entity));

    ui_sprite_fsm_action_sync_update(fsm_action, 0);
}

static void ui_sprite_2d_move_on_set_to(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_2d_move_t move = ctx;
    ui_sprite_2d_module_t module = move->m_module;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    UI_SPRITE_EVT_2D_MOVE_SET_TO const * evt_data = evt->data;

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move on set to: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
        return;
    }

    ui_sprite_2d_transform_set_pos(transform, evt_data->pos);
    ui_sprite_fsm_action_sync_update(fsm_action, 0);
}

int ui_sprite_2d_move_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_2d_module_t module = move->m_module;

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_2d_move_to", ui_sprite_2d_move_on_move_to, move) != 0)
    {
        CPE_ERROR(module->m_em, "camera move enter: add eventer handler fail!");
        return -1;
    }

	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self,
		"ui_sprite_evt_2d_move_follow_entity", ui_sprite_2d_move_on_follow_entity, move) != 0)
	{
		CPE_ERROR(module->m_em, "camera move enter: add eventer handler fail!");
		return -1;
	}

	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self,
		"ui_sprite_evt_2d_move_set_to_entity", ui_sprite_2d_move_on_set_to_entity, move) != 0)
	{
		CPE_ERROR(module->m_em, "camera move enter: add eventer handler fail!");
		return -1;
	}
 
	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self, 
		"ui_sprite_evt_2d_move_set_to", ui_sprite_2d_move_on_set_to, move) != 0)
	{    
		CPE_ERROR(module->m_em, "camera move enter: add eventer handler fail!");
		return -1;
	}
	
    return 0;
}

void ui_sprite_2d_move_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_move_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_move_t move_to = ui_sprite_fsm_action_data(fsm_action);

    bzero(move_to, sizeof(*move_to));

    move_to->m_module = ctx;

    return 0;
}

int ui_sprite_2d_move_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_move_init(to, ctx);
    return 0;
}

void ui_sprite_2d_move_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

void ui_sprite_2d_move_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
	ui_sprite_2d_move_t move = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_2d_module_t module = move->m_module;
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
	UI_SPRITE_2D_PAIR cur_pos = ui_sprite_2d_transform_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN);
	UI_SPRITE_2D_PAIR set_to_pos;

    if (move->m_follow_entity_id != 0 || move->m_follow_entity_name[0]) {
        ui_sprite_entity_t set_to_entity = NULL;
        ui_sprite_2d_transform_t set_to_transform;

        if (move->m_follow_entity_id > 0) {
            set_to_entity = ui_sprite_entity_find_by_id(ui_sprite_entity_world(entity), move->m_follow_entity_id);
            if (set_to_entity == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): 2d move: update: entity %d not exist!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move->m_follow_entity_id);
                ui_sprite_fsm_action_sync_update(fsm_action, 0);
                return;
            }
        }
        else {
            set_to_entity = ui_sprite_entity_find_by_name(ui_sprite_entity_world(entity), move->m_follow_entity_name);
            if (set_to_entity == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): 2d move: update: entity %s not exist!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move->m_follow_entity_name);
                ui_sprite_fsm_action_sync_update(fsm_action, 0);
                return;
            }
        }

        set_to_transform = ui_sprite_2d_transform_find(set_to_entity);
        if (set_to_transform == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): 2d move: update: follow entity %d(%s) no transform!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), 
                ui_sprite_entity_id(set_to_entity), ui_sprite_entity_name(set_to_entity));
            ui_sprite_fsm_action_sync_update(fsm_action, 0);
            return;
        }

        move->m_target_pos = ui_sprite_2d_transform_pos(set_to_transform, move->m_follow_entity_pos);
    }

    if (move->m_duration > 0.0f) {
        if (move->m_duration > delta) {
            float percent = delta / move->m_duration;
            set_to_pos.x = cur_pos.x + (move->m_target_pos.x - cur_pos.x) * percent;
            set_to_pos.y = cur_pos.y + (move->m_target_pos.y - cur_pos.y) * percent;
        }
        else {
            set_to_pos = move->m_target_pos;
        }

        move->m_duration -= delta;
    }
    else if (move->m_speed > 0.0f) {
        //TODO: Beik
        set_to_pos = move->m_target_pos;
    }
    else {
        set_to_pos = move->m_target_pos;
    }

    //printf("update pos (%f,%f) ==> (%f,%f)\n", cur_pos.x, cur_pos.y, set_to_pos.x, set_to_pos.y);

    ui_sprite_2d_transform_set_pos(transform, set_to_pos);

    if ((move->m_follow_entity_id == 0 && move->m_follow_entity_name[0] == 0)
        && fabs(set_to_pos.x - move->m_target_pos.x) < 0.01f
        && fabs(set_to_pos.y-  move->m_target_pos.y) < 0.01f
        )
    {
        ui_sprite_fsm_action_sync_update(fsm_action, 0);
    }
}


int ui_sprite_2d_move_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_MOVE_NAME, sizeof(struct ui_sprite_2d_move));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_MOVE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_move_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_move_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_move_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_move_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_move_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_move_update, module);

    return 0;
}

void ui_sprite_2d_move_unregist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_MOVE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_MOVE_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_2D_MOVE_NAME = "2d-move";
