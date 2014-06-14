#include  <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_anim_camera_scale_i.h"
#include "ui_sprite_anim_camera_i.h"
#include "ui_sprite_anim_camera_op_i.h"

ui_sprite_anim_camera_scale_t ui_sprite_anim_camera_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_CAMERA_SCALE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_camera_scale_free(ui_sprite_anim_camera_scale_t scale) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(scale);
    ui_sprite_fsm_action_free(fsm_action);
}

int8_t ui_sprite_anim_camera_scale_priority(ui_sprite_anim_camera_scale_t scale) {
    return scale->m_priority;
}

void ui_sprite_anim_camera_scale_set_priority(ui_sprite_anim_camera_scale_t scale, int8_t priority) {
    scale->m_priority = priority;
}

static void ui_sprite_anim_camera_scale_on_scale(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_scale_t scale = ctx;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
	UI_SPRITE_EVT_ANIM_CAMERA_SCALE const * evt_data = evt->data;

    scale->m_target_scale = evt_data->scale;
    scale->m_pos_on_screen.x = evt_data->keep_pos_on_screen.x;
    scale->m_pos_on_screen.y = evt_data->keep_pos_on_screen.y;
    scale->m_pos_in_world = ui_sprite_anim_camera_screen_to_world(camera, scale->m_pos_on_screen);
    scale->m_duration = evt_data->duration;

    ui_sprite_fsm_action_sync_update(action, 1);
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

static void ui_sprite_anim_camera_scale_do_clear(ui_sprite_anim_camera_t camera, ui_sprite_anim_camera_scale_t scale) {
    if (scale->m_curent_op_id != UI_SPRITE_INVALID_CAMERA_OP_ID) {
        ui_sprite_anim_camera_op_stop(camera, scale->m_curent_op_id);
        scale->m_curent_op_id = UI_SPRITE_INVALID_CAMERA_OP_ID;
    }
}

static void ui_sprite_anim_camera_scale_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera scale exit: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (scale->m_curent_op_id != UI_SPRITE_INVALID_CAMERA_OP_ID) {
        ui_sprite_anim_camera_op_stop(camera, scale->m_curent_op_id);
        scale->m_curent_op_id = UI_SPRITE_INVALID_CAMERA_OP_ID;
    }
}

static void ui_sprite_anim_camera_scale_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_anim_camera_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
    float target_scale;
    UI_SPRITE_ANIM_CAMERA_OP op_data;

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera update: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_anim_camera_scale_do_clear(camera, scale);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (scale->m_duration > delta) {
        target_scale = camera->m_camera_scale + (scale->m_target_scale - camera->m_camera_scale) * delta / scale->m_duration;
        scale->m_duration -= delta;
    }
    else {
        target_scale = scale->m_target_scale;
        scale->m_duration = 0.0f;
    }

    op_data.type = UI_SPRITE_ANIM_CAMERA_OP_TYPE_MOVE_TO_TARGET;
    op_data.data.move_to_target.target_scale = target_scale;
    op_data.data.move_to_target.target_pos.x = scale->m_pos_in_world.x - scale->m_pos_on_screen.x * target_scale;
    op_data.data.move_to_target.target_pos.y = scale->m_pos_in_world.y - scale->m_pos_on_screen.y * target_scale;
    op_data.data.move_to_target.max_speed = 0;
    op_data.data.move_to_target.duration = 0;

    scale->m_curent_op_id =
        ui_sprite_anim_camera_op_check_start(
            camera,
            scale->m_curent_op_id,
            &op_data,
            scale->m_priority,
            ui_sprite_anim_camera_op_suspend_remove,
            ui_sprite_anim_camera_op_complete_remove);

    if (scale->m_duration <= 0.0f) {
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

    assert (scale->m_curent_op_id == UI_SPRITE_INVALID_CAMERA_OP_ID);
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

