#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_wait_switchback.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_2d_wait_switchback(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_cfg_loader_t loader = ctx;
	ui_sprite_2d_wait_switchback_t p2d_wait_switchback = ui_sprite_2d_wait_switchback_create(fsm_state, name);
    const char * track_pos;

	if (p2d_wait_switchback == NULL) {
		CPE_ERROR(loader->m_em, "%s: create anim_2d_wait_switchback action: create fail!", ui_sprite_cfg_loader_name(loader));
		return NULL;
	}

    if ((track_pos = cfg_get_string(cfg, "track-pos", NULL))) {
        uint8_t pos_policy = ui_sprite_2d_transform_pos_policy_from_str(track_pos);
        if (pos_policy == 0) {
            CPE_ERROR(
                loader->m_em, "%s: create anim_2d_wait_switchback action: track-pos %s is unknown!",
                ui_sprite_cfg_loader_name(loader), track_pos);
            ui_sprite_2d_wait_switchback_free(p2d_wait_switchback);
            return NULL;
        }

        ui_sprite_2d_wait_switchback_set_pos(p2d_wait_switchback, pos_policy);
    }

    ui_sprite_2d_wait_switchback_set_process_x(
        p2d_wait_switchback,
        cfg_get_uint8(cfg, "process-x", ui_sprite_2d_wait_switchback_process_x(p2d_wait_switchback)));

    ui_sprite_2d_wait_switchback_set_process_y(
        p2d_wait_switchback,
        cfg_get_uint8(cfg, "process-y", ui_sprite_2d_wait_switchback_process_y(p2d_wait_switchback)));

	return ui_sprite_fsm_action_from_data(p2d_wait_switchback);
}

