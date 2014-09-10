#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_move_follow.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_2d_move_follow(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_cfg_loader_t loader = ctx;
	ui_sprite_2d_move_follow_t p2d_move_follow = ui_sprite_2d_move_follow_create(fsm_state, name);

	if (p2d_move_follow == NULL) {
		CPE_ERROR(loader->m_em, "%s: create anim_2d_move_follow action: create fail!", ui_sprite_cfg_loader_name(loader));
		return NULL;
	}

	return ui_sprite_fsm_action_from_data(p2d_move_follow);
}

