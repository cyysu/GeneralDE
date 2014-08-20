#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_search.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_2d_search(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_cfg_loader_t loader = ctx;
	ui_sprite_2d_search_t search = ui_sprite_2d_search_create(fsm_state, name);

	if (search == NULL) {
		CPE_ERROR(loader->m_em, "%s: create anim_2d_search action: create fail!", ui_sprite_cfg_loader_name(loader));
		return NULL;
	}

    if (ui_sprite_2d_search_set_on_found(search, cfg_get_string(cfg, "on-found", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create 2d search action: set on-found fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_2d_search_free(search);
        return NULL;
    }

	return ui_sprite_fsm_action_from_data(search);
}

