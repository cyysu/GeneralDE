#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_move.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_2d_move(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_cfg_loader_t loader = ctx;
	ui_sprite_2d_move_t p2d_move = ui_sprite_2d_move_create(fsm_state, name);
    const char * decorator;

	if (p2d_move == NULL) {
		CPE_ERROR(loader->m_em, "%s: create anim_2d_move action: create fail!", ui_sprite_cfg_loader_name(loader));
		return NULL;
	}

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_2d_move_contain_set_decorator(p2d_move, decorator) != 0) {
            CPE_ERROR(loader->m_em, "%s: create 2d move action: create fail!", ui_sprite_cfg_loader_name(loader));
            ui_sprite_2d_move_free(p2d_move);
            return NULL;
        }
    }

	return ui_sprite_fsm_action_from_data(p2d_move);
}

