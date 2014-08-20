#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_anim_camera_move_i.h"
#include "ui_sprite_anim_camera_i.h"

ui_sprite_anim_camera_move_t ui_sprite_anim_camera_move_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_CAMERA_MOVE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_camera_move_free(ui_sprite_anim_camera_move_t move) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_anim_camera_move_set_decorator(ui_sprite_anim_camera_move_t move, const char * decorator) {
    return ui_percent_decorator_setup(&move->m_updator.m_decorator, decorator, move->m_module->m_em);
}

static UI_SPRITE_2D_PAIR
ui_sprite_anim_camera_move_target_camera_pos(
ui_sprite_anim_camera_move_t move, ui_sprite_anim_camera_t camera,
    UI_SPRITE_2D_PAIR const * pos_in_screen, UI_SPRITE_2D_PAIR const * pos_in_world, float scale)
{
    UI_SPRITE_2D_PAIR pos;

    pos.x = pos_in_world->x - camera->m_screen_size.x * scale * pos_in_screen->x;
    pos.y = pos_in_world->y - camera->m_screen_size.y * scale * pos_in_screen->y;

    return pos;
}

static void ui_sprite_anim_camera_move_on_move_to_entity(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_move_t move = ctx;
    ui_sprite_anim_module_t module = move->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
	UI_SPRITE_EVT_ANIM_CAMERA_MOVE_TO_ENTITY const * evt_data = evt->data;
    uint8_t pos_of_entity;
    UI_SPRITE_2D_PAIR entity_pos_in_screen;
    UI_SPRITE_2D_PAIR entity_pos_in_world;
    UI_SPRITE_2D_PAIR target_pos;
    float target_scale;

    assert(camera->m_camera_scale > 0.0f);

    ui_sprite_anim_camera_updator_stop(&move->m_updator, camera);

    pos_of_entity = ui_sprite_2d_transform_pos_policy_from_str(evt_data->pos_of_entity);
    if (pos_of_entity == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on move to entity: pos of entity %s is unknown!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_of_entity);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    target_scale = evt_data->scale == 0.0f ? camera->m_camera_scale : evt_data->scale;

    if (ui_sprite_anim_camera_pos_of_entity(&entity_pos_in_world, world, evt_data->entity_id, evt_data->entity_name, pos_of_entity) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on set to entity: get pos of entity %d(%s) fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->entity_id, evt_data->entity_name);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    entity_pos_in_screen.x = evt_data->pos_in_screen.x;
    entity_pos_in_screen.y = evt_data->pos_in_screen.y;

    target_pos =
        ui_sprite_anim_camera_move_target_camera_pos(
            move, camera, &entity_pos_in_screen, &entity_pos_in_world, target_scale);

    ui_sprite_anim_camera_adj_camera_in_limit(camera, &target_pos, &target_scale);

    ui_sprite_anim_camera_updator_set_max_speed(&move->m_updator, evt_data->max_speed);

    ui_sprite_anim_camera_updator_set_camera(&move->m_updator, camera, target_pos, target_scale);

    ui_sprite_fsm_action_sync_update(action, ui_sprite_anim_camera_updator_is_runing(&move->m_updator));
}

static void ui_sprite_anim_camera_move_on_move_to_group(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_move_t move = ctx;
    ui_sprite_anim_module_t module = move->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
	UI_SPRITE_EVT_ANIM_CAMERA_MOVE_TO_GROUP const * evt_data = evt->data;
    UI_SPRITE_2D_PAIR world_lt = { 0.0f, 0.0f };
    UI_SPRITE_2D_PAIR world_rb = { 0.0f, 0.0f };
	ui_sprite_group_t target_group;
    int entity_count = 0;
    UI_SPRITE_2D_PAIR center_pos_in_screen;
    UI_SPRITE_2D_PAIR center_pos_in_world;
    UI_SPRITE_2D_PAIR target_pos;
    float target_scale;

    assert(camera->m_camera_scale > 0.0f);

    ui_sprite_anim_camera_updator_stop(&move->m_updator, camera);

    target_group = ui_sprite_group_find_by_name(world,  evt_data->group_name);
    if (target_group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on move to group: group %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->group_name);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    entity_count = ui_sprite_2d_merge_contain_rect_group(&world_lt, &world_rb, target_group);
    if (entity_count < 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on move to group: group %s get entities fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->group_name);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }
    else if (entity_count == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera on move to group: group %s: no entity!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->group_name);
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    target_scale = evt_data->scale == 0.0f ? camera->m_camera_scale : evt_data->scale;
    if ((world_rb.x - world_lt.x) * target_scale < (evt_data->group_screen_rb.x - evt_data->group_screen_lt.x)) {
        target_scale = (evt_data->group_screen_rb.x - evt_data->group_screen_lt.x) / (world_rb.x - world_lt.x);
    }

    if ((world_rb.y - world_lt.y) * target_scale < (evt_data->group_screen_rb.y - evt_data->group_screen_lt.y)) {
        target_scale = (evt_data->group_screen_rb.y - evt_data->group_screen_lt.y) / (world_rb.y - world_lt.y);
    }
    
    center_pos_in_world.x = (world_lt.x + world_rb.x) / 2.0f;
    center_pos_in_world.y = (world_lt.y + world_rb.y) / 2.0f;
    center_pos_in_screen.x = (evt_data->group_screen_lt.x + evt_data->group_screen_rb.x) / 2.0f;
    center_pos_in_screen.y = (evt_data->group_screen_lt.y + evt_data->group_screen_rb.y) / 2.0f;

    target_pos =
        ui_sprite_anim_camera_move_target_camera_pos(
            move, camera, &center_pos_in_screen, &center_pos_in_world, target_scale);

    ui_sprite_anim_camera_adj_camera_in_limit(camera, &target_pos, &target_scale);

    ui_sprite_anim_camera_updator_set_max_speed(&move->m_updator, evt_data->max_speed);

    ui_sprite_anim_camera_updator_set_camera(&move->m_updator, camera, target_pos, target_scale);

    ui_sprite_fsm_action_sync_update(action, ui_sprite_anim_camera_updator_is_runing(&move->m_updator));
}

static void ui_sprite_anim_camera_move_on_move_to_pos(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_move_t move = ctx;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
	UI_SPRITE_EVT_ANIM_CAMERA_MOVE_TO_POS const * evt_data = evt->data;
    UI_SPRITE_2D_PAIR target_pos_in_screen;
    UI_SPRITE_2D_PAIR target_pos_in_world;
    UI_SPRITE_2D_PAIR camera_target_pos;
    float camera_target_scale;

    assert(camera->m_camera_scale > 0.0f);

    ui_sprite_anim_camera_updator_stop(&move->m_updator, camera);

    camera_target_scale = evt_data->scale == 0.0f ? camera->m_camera_scale : evt_data->scale;

    target_pos_in_world.x = evt_data->pos_in_world.x;
    target_pos_in_world.y = evt_data->pos_in_world.y;
    target_pos_in_screen.x = evt_data->pos_in_screen.x;
    target_pos_in_screen.y = evt_data->pos_in_screen.y;

    camera_target_pos =
        ui_sprite_anim_camera_move_target_camera_pos(
            move, camera, &target_pos_in_screen, &target_pos_in_world, camera_target_scale);

    ui_sprite_anim_camera_adj_camera_in_limit(camera, &camera_target_pos, &camera_target_scale);

    ui_sprite_anim_camera_updator_set_max_speed(&move->m_updator, evt_data->max_speed);

    ui_sprite_anim_camera_updator_set_camera(&move->m_updator, camera, camera_target_pos, camera_target_scale);

    ui_sprite_fsm_action_sync_update(action, ui_sprite_anim_camera_updator_is_runing(&move->m_updator));
}

static int ui_sprite_anim_camera_move_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_anim_module_t module = ctx;

	if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_anim_camera_move_to_entity", ui_sprite_anim_camera_move_on_move_to_entity, move) != 0
        || ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_anim_camera_move_to_group", ui_sprite_anim_camera_move_on_move_to_group, move) != 0
        || ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_anim_camera_move_to_pos", ui_sprite_anim_camera_move_on_move_to_pos, move) != 0
        )
	{    
		CPE_ERROR(module->m_em, "camera move enter: add eventer handler fail!");
		return -1;
	}

    return 0;
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

    ui_sprite_anim_camera_updator_stop(&move->m_updator, camera);
}

static void ui_sprite_anim_camera_move_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_anim_camera_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera move: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    ui_sprite_anim_camera_updator_update(&move->m_updator, camera, delta);

    if (!ui_sprite_anim_camera_updator_is_runing(&move->m_updator)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
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

    assert(move->m_updator.m_curent_op_id == 0);
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

