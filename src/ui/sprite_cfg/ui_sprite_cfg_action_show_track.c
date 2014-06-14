#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_show_track.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_show_track(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_cfg_loader_t loader = ctx;
	ui_sprite_anim_show_track_t show_track = ui_sprite_anim_show_track_create(fsm_state, name);

	if (show_track == NULL) {
		CPE_ERROR(loader->m_em, "%s: create anim_show_track action: create fail!", ui_sprite_cfg_loader_name(loader));
		return NULL;
	}

	return ui_sprite_fsm_action_from_data(show_track);
}

