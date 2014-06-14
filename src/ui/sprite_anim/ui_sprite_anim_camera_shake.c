#include<math.h>
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_anim_camera_shake_i.h"
#include "ui_sprite_anim_camera_i.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"

ui_sprite_anim_camera_shake_t ui_sprite_anim_camera_shake_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_CAMERA_SHAKE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_camera_shake_free(ui_sprite_anim_camera_shake_t shake) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(shake);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_anim_camera_shake_on_shake_to(void * ctx, ui_sprite_event_t evt) {
	ui_sprite_anim_camera_shake_t shake = ctx;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);  
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
	UI_SPRITE_EVT_ANIM_CAMERA_SHAKE_TO const * evt_data = evt->data;

	shake->m_num = evt_data->num;
	if(shake->m_num == 0 || (evt_data->amplitude.x ==0 && evt_data->amplitude.y == 0)){
		return;
	}

	shake->m_onceDuration = evt_data->duration / shake->m_num;
	shake->m_speed.x = evt_data->amplitude.x / (shake->m_onceDuration / 4.0f);
	shake->m_speed.y = evt_data->amplitude.y / (shake->m_onceDuration / 4.0f);
	shake->m_work_duration = 0.0f;
	shake->m_amplitude_count = 0;
	shake->m_camera_pos = ui_sprite_anim_camera_pos(camera);
	ui_sprite_fsm_action_sync_update(ui_sprite_fsm_action_from_data(shake), 1);
}

static int ui_sprite_anim_camera_shake_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_shake_t shake = ui_sprite_fsm_action_data(fsm_action); 
    ui_sprite_anim_module_t module = ctx;

	if (ui_sprite_fsm_action_add_event_handler(
		fsm_action, ui_sprite_event_scope_self, 
		"ui_sprite_evt_anim_camera_shake_to", ui_sprite_anim_camera_shake_on_shake_to, shake) != 0)
	{    
		CPE_ERROR(module->m_em, "camera shake enter: add eventer handler fail!");
		return -1;
	}
    return 0;
}

static void ui_sprite_anim_camera_shake_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {

}

static int ui_sprite_anim_camera_shake_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_camera_shake_t shake = ui_sprite_fsm_action_data(fsm_action);
    shake->m_module = ctx;
	shake->m_speed.x = 0.0f;
	shake->m_speed.y = 0.0f;
	shake->m_num = 0;
	shake->m_onceDuration= 0.0f;
	shake->m_work_duration= 0.0f;
	shake->m_camera_pos.x =0.0f;
	shake->m_camera_pos.y =0.0f;
	shake->m_amplitude_count =0;
    return 0;
}

static void ui_sprite_anim_camera_shake_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_anim_camera_shake_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_anim_camera_shake_init(to, ctx)) return -1;

    return 0;
}

static void ui_sprite_anim_camera_shake_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
	ui_sprite_anim_camera_shake_t shake = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
	UI_SPRITE_2D_PAIR camera_pos = ui_sprite_anim_camera_pos(camera);
	if(shake->m_speed.x != 0.0f){
		if(shake->m_amplitude_count % 2 == 0)
			camera_pos.x += shake->m_speed.x * delta;
		else 
			camera_pos.x -= shake->m_speed.x * delta;
	}

	if(shake->m_speed.y != 0.0f){
		if(shake->m_amplitude_count % 2 == 0)
			camera_pos.y += shake->m_speed.y * delta;
		else 
			camera_pos.y -= shake->m_speed.y * delta;
	}

	ui_sprite_anim_camera_set_pos_and_scale(camera, camera_pos, camera->m_camera_scale);
	shake->m_work_duration += delta;
	if(shake->m_work_duration >= shake->m_onceDuration / 4.0f){
		shake->m_amplitude_count++;
	}
	//CPE_ERROR( module->m_em, "shake ==== %d", shake->m_num);
	if(shake->m_amplitude_count >= 4){
		shake->m_num --;
		if(shake->m_num == 0){
			ui_sprite_anim_camera_set_pos_and_scale(camera, shake->m_camera_pos, camera->m_camera_scale);
			ui_sprite_fsm_action_stop_update(fsm_action);
		}
	}
}
int ui_sprite_anim_camera_shake_regist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_SHAKE_NAME, sizeof(struct ui_sprite_anim_camera_shake));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim camera shake register: meta create fail",
            ui_sprite_anim_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_anim_camera_shake_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_anim_camera_shake_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_anim_camera_shake_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_anim_camera_shake_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_anim_camera_shake_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_anim_camera_shake_update, module);

    return 0;
}

void ui_sprite_anim_camera_shake_unregist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_SHAKE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim camera shake unregister: meta not exist",
            ui_sprite_anim_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_ANIM_CAMERA_SHAKE_NAME = "camera-shake";

