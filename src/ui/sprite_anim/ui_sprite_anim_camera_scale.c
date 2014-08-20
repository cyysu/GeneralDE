#include  <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_anim_camera_scale_i.h"
#include "ui_sprite_anim_camera_utils.h"

ui_sprite_anim_camera_scale_t ui_sprite_anim_camera_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_CAMERA_SCALE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_camera_scale_free(ui_sprite_anim_camera_scale_t scale) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(scale);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_anim_camera_scale_set_decorator(ui_sprite_anim_camera_scale_t scale, const char * decorator) {
    return ui_percent_decorator_setup(&scale->m_updator.m_decorator, decorator, scale->m_module->m_em);
}

static void ui_sprite_anim_camera_scale_on_scale(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_scale_t scale = ctx;
    ui_sprite_anim_module_t module = scale->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
	UI_SPRITE_EVT_ANIM_CAMERA_SCALE const * evt_data = evt->data;
    UI_SPRITE_2D_PAIR target_pos;
    float target_scale;
    ui_sprite_entity_t lock_entity = NULL;

    ui_sprite_anim_camera_updator_stop(&scale->m_updator, camera);

    if (camera->m_camera_scale == evt_data->scale) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): camera on scale: no scale change, scale=%f!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), camera->m_camera_scale);
        }
        ui_sprite_fsm_action_sync_update(action, 0);
        return;
    }

    target_scale = evt_data->scale;

    if (evt_data->lock_entity_id) {
        lock_entity = ui_sprite_entity_find_by_id(world, evt_data->lock_entity_id);
        if (lock_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera on scale: lock entity %d not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->lock_entity_id);
            ui_sprite_fsm_action_sync_update(action, 0);
            return;
        }
    }
    else if (evt_data->lock_entity_name[0]) {
        lock_entity = ui_sprite_entity_find_by_name(world, evt_data->lock_entity_name);
        if (lock_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera on scale: lock entity %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->lock_entity_name);
            ui_sprite_fsm_action_sync_update(action, 0);
            return;
        }
    }

    if (lock_entity) {
        ui_sprite_2d_transform_t transform;
        UI_SPRITE_2D_PAIR pos_in_screen;
        UI_SPRITE_2D_PAIR pos_in_world;

        transform = ui_sprite_2d_transform_find(lock_entity);
        if (transform == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera on scale: lock entity %d(%s) no transform!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_id(lock_entity), ui_sprite_entity_name(lock_entity));
            ui_sprite_fsm_action_sync_update(action, 0);
            return;
        }

        pos_in_world = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN, 0);

        pos_in_screen.x = (pos_in_world.x - camera->m_camera_pos.x) / (camera->m_screen_size.x * camera->m_camera_scale);
        pos_in_screen.y = (pos_in_world.y - camera->m_camera_pos.y) / (camera->m_screen_size.y * camera->m_camera_scale);

        if (!ui_sprite_2d_pt_in_rect(pos_in_screen, &g_full_screen_percent)) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): camera on scale: lock pos (%f,%f) is not in screen!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                pos_in_screen.x, pos_in_screen.y);
            ui_sprite_fsm_action_sync_update(action, 0);
            return;
        }

        target_pos.x = 
            camera->m_camera_pos.x
            + (camera->m_screen_size.x * (camera->m_camera_scale - target_scale) * pos_in_screen.x);

        target_pos.y =
            camera->m_camera_pos.y
            + (camera->m_screen_size.y * (camera->m_camera_scale - target_scale) * pos_in_screen.y);

        ui_sprite_anim_camera_adj_camera_in_limit_with_lock_pos(
            camera, &target_pos, &target_scale, &pos_in_screen, &pos_in_world);
    }
    else {
        target_pos = camera->m_camera_pos;
        ui_sprite_anim_camera_adj_camera_in_limit(camera, &target_pos, &target_scale);
    }

    ui_sprite_anim_camera_updator_set_max_speed(&scale->m_updator, evt_data->max_speed);

    ui_sprite_anim_camera_updator_set_camera(&scale->m_updator, camera, target_pos, target_scale);

    ui_sprite_fsm_action_sync_update(action, ui_sprite_anim_camera_updator_is_runing(&scale->m_updator));
}

static int ui_sprite_anim_camera_scale_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_anim_module_t module = ctx;

	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self, 
		"ui_sprite_evt_anim_camera_scale", ui_sprite_anim_camera_scale_on_scale, scale) != 0)
	{    
		CPE_ERROR(module->m_em, "camera scale enter: add eventer handler fail!");
		return -1;
	}
	
    return 0;
}

static void ui_sprite_anim_camera_scale_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);

    ui_sprite_anim_camera_updator_stop(&scale->m_updator, camera);
}

static void ui_sprite_anim_camera_scale_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_anim_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera update: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_anim_camera_updator_stop(&scale->m_updator, camera);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    ui_sprite_anim_camera_updator_update(&scale->m_updator, camera, delta);

    if (!ui_sprite_anim_camera_updator_is_runing(&scale->m_updator)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static int ui_sprite_anim_camera_scale_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);

    bzero(scale, sizeof(*scale));

    scale->m_module = ctx;

    return 0;
}

static void ui_sprite_anim_camera_scale_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);

    assert (scale->m_updator.m_curent_op_id == 0);
}

static int ui_sprite_anim_camera_scale_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_anim_camera_scale_init(to, ctx)) return -1;
    return 0;
}

int ui_sprite_anim_camera_scale_regist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_SCALE_NAME, sizeof(struct ui_sprite_anim_camera_scale));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim camera scale register: meta create fail",
            ui_sprite_anim_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_anim_camera_scale_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_anim_camera_scale_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_anim_camera_scale_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_anim_camera_scale_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_anim_camera_scale_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_anim_camera_scale_update, module);

    return 0;
}

void ui_sprite_anim_camera_scale_unregist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_SCALE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim camera scale unregister: meta not exist",
            ui_sprite_anim_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_ANIM_CAMERA_SCALE_NAME = "camera-scale";

