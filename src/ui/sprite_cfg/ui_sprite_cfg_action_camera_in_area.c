#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_in_area.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_camera_in_area(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_cfg_loader_t loader = ctx;
	ui_sprite_anim_camera_in_area_t in_area = ui_sprite_anim_camera_in_area_create(fsm_state, name);

	if (in_area == NULL) {
		CPE_ERROR(loader->m_em, "%s: create anim_in_area action: create fail!", ui_sprite_cfg_loader_name(loader));
		return NULL;
	}

	return ui_sprite_fsm_action_from_data(in_area);
}

