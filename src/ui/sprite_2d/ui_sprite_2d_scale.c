#include "cpe/pal/pal_strings.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_scale.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_scale_i.h"
#include "ui_sprite_2d_module_i.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_evt.h"
#include "ui/utils//ui_percent_decorator.h"

ui_sprite_2d_scale_t
ui_sprite_2d_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_SCALE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_scale_free(ui_sprite_2d_scale_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

static void ui_sprite_2d_scale_scale(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_2d_scale_t scale = ctx;
    UI_SPRITE_EVT_2D_SCALE const * evt_data = evt->data;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    scale->m_origin_scale = ui_sprite_2d_transform_scale_pair(transform);
    scale->m_work_duration = evt_data->duration;
	scale->m_target_scale = evt_data->scale;
    scale->m_runing_time = 0.0f;
	ui_sprite_fsm_action_sync_update(ui_sprite_fsm_action_from_data(scale), 1);
}

int ui_sprite_2d_scale_contain_set_decorator(ui_sprite_2d_scale_t show_anim, const char * decorator) {
    return ui_percent_decorator_setup(&show_anim->m_decorator, decorator, show_anim->m_module->m_em);
}

int ui_sprite_2d_scale_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_2d_module_t module = scale->m_module;

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_2d_scale", ui_sprite_2d_scale_scale, scale) != 0)
    {
        CPE_ERROR(module->m_em, "camera scale enter: add eventer handler fail!");
        return -1;
	}	
    return 0;
}

void ui_sprite_2d_scale_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
	ui_sprite_2d_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
	float percent = 0.0;
    UI_SPRITE_2D_PAIR curScale;
    scale->m_runing_time += delta;

    percent = scale->m_runing_time >= scale->m_work_duration ? 1.0f : scale->m_runing_time / scale->m_work_duration;
    percent = ui_percent_decorator_decorate(&scale->m_decorator, percent);
    curScale.x = scale->m_origin_scale.x + (scale->m_target_scale.x - scale->m_origin_scale.x) * percent;
    curScale.y = scale->m_origin_scale.y + (scale->m_target_scale.y - scale->m_origin_scale.y) * percent;

    if (scale->m_runing_time >= scale->m_work_duration) {
        ui_sprite_2d_transform_set_scale_pair(transform, scale->m_target_scale);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
    ui_sprite_2d_transform_set_scale_pair(transform, curScale);
}


void ui_sprite_2d_scale_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {

}

int ui_sprite_2d_scale_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_scale_t scale = ui_sprite_fsm_action_data(fsm_action);

	bzero(scale, sizeof(*scale));
	scale->m_module = ctx;
	scale->m_work_duration = 0.0f;
	return 0;
}

int ui_sprite_2d_scale_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_scale_t to_scale_to = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_scale_t from_scale_to = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_scale_init(to, ctx);
    to_scale_to->m_max_speed = from_scale_to->m_max_speed;
    memcpy(&to_scale_to->m_decorator, &from_scale_to->m_decorator, sizeof(to_scale_to->m_decorator));
    return 0;
}

void ui_sprite_2d_scale_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_scale_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_SCALE_NAME, sizeof(struct ui_sprite_2d_scale));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_SCALE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_scale_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_scale_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_scale_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_scale_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_scale_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_scale_update, module);

    return 0;
}

void ui_sprite_2d_scale_unregist(ui_sprite_2d_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_SCALE_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_2d_module_name(module), UI_SPRITE_2D_SCALE_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_2D_SCALE_NAME = "2d-scale";
