#include <assert.h>
#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_anim_camera_in_area_i.h"
#include "ui_sprite_anim_camera_i.h"
#include "ui_sprite_anim_camera_op_i.h"
#include "ui_sprite_anim_camera_restrict_i.h"

ui_sprite_anim_camera_in_area_t ui_sprite_anim_camera_in_area_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
	ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_CAMERA_IN_AREA_NAME);
	return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_camera_in_area_free(ui_sprite_anim_camera_in_area_t in_area) {
	ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(in_area);
	ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_anim_camera_in_area_do_restrict(void * ctx, ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR * pos, float * scale) {
	ui_sprite_anim_camera_in_area_t in_area = ctx;
	ui_sprite_anim_module_t module = in_area->m_module;
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(in_area));

	UI_SPRITE_2D_PAIR camera_size = camera->m_screen_size;
	UI_SPRITE_2D_PAIR camera_limit_lt = camera->m_limit_lt;
	UI_SPRITE_2D_PAIR camera_limit_rb = camera->m_limit_rb;
	uint8_t outScreen_x = 0;
	uint8_t outScreen_y = 0;
	float scale_x = 0.0f;
	float scale_y = 0.0f;

	if (camera == NULL) {
		CPE_ERROR(
			module->m_em, "entity %d(%s): camera update: no camera!",
			ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
		return;
	}

	if((*pos).x < camera_limit_lt.x) {
		(*pos).x = camera_limit_lt.x;
    }

	if((*pos).y < camera_limit_lt.y) {
		(*pos).y = camera_limit_lt.y;
    }

	outScreen_x = (*pos).x + camera_size.x * (*scale) > camera_limit_rb.x;
	outScreen_y = (*pos).y + camera_size.y * (*scale) > camera_limit_rb.y;

	if(outScreen_x && outScreen_x){
		scale_x = fabs((*pos).x - camera_limit_rb.x) / camera_size.x;
		scale_y = fabs((*pos).y - camera_limit_rb.y) / camera_size.y;
		(*scale) = scale_x > scale_y ? scale_y : scale_x;
	}
	else if(outScreen_x){
		*scale = fabs((*pos).x - camera_limit_rb.x) / camera_size.x;
	}
	else if(outScreen_y){
		*scale = fabs((*pos).y - camera_limit_rb.y) / camera_size.y;
	}
}

static int ui_sprite_anim_camera_in_area_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_anim_camera_in_area_t in_area = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_anim_module_t module = ctx;
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);

	assert(in_area->m_restrict == NULL);
	in_area->m_restrict = ui_sprite_anim_camera_restrict_create(
		camera,
		UI_SPRITE_ANIM_CAMERA_IN_AREA_NAME,
		ui_sprite_anim_camera_in_area_do_restrict,
		in_area);
	if (in_area->m_restrict == NULL) {
		CPE_ERROR(module->m_em, "create camera in area fail!");
		return -1;
	}

	return 0;
}

static void ui_sprite_anim_camera_in_area_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_anim_camera_in_area_t in_area = ui_sprite_fsm_action_data(fsm_action);
	assert(in_area->m_restrict);
	ui_sprite_anim_camera_restrict_free(in_area->m_restrict);
	in_area->m_restrict = NULL;
}

static int ui_sprite_anim_camera_in_area_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_anim_camera_in_area_t in_area = ui_sprite_fsm_action_data(fsm_action);

	bzero(in_area, sizeof(*in_area));

	in_area->m_module = ctx;

	return 0;
}

static void ui_sprite_anim_camera_in_area_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_anim_camera_in_area_t in_area = ui_sprite_fsm_action_data(fsm_action);
	assert(in_area->m_restrict == NULL);
}

static int ui_sprite_anim_camera_in_area_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
	if (ui_sprite_anim_camera_in_area_init(to, ctx)) return -1;
	return 0;
}

int ui_sprite_anim_camera_in_area_regist(ui_sprite_anim_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_IN_AREA_NAME, sizeof(struct ui_sprite_anim_camera_in_area));
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: anim camera in_area register: meta create fail",
			ui_sprite_anim_module_name(module));
		return -1;
	}

	ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_anim_camera_in_area_enter, module);
	ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_anim_camera_in_area_exit, module);
	ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_anim_camera_in_area_init, module);
	ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_anim_camera_in_area_copy, module);
	ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_anim_camera_in_area_clear, module);

	return 0;
}

void ui_sprite_anim_camera_in_area_unregist(ui_sprite_anim_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_ANIM_CAMERA_IN_AREA_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: anim camera in_area unregister: meta not exist",
			ui_sprite_anim_module_name(module));
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_ANIM_CAMERA_IN_AREA_NAME = "camera-in-area";

