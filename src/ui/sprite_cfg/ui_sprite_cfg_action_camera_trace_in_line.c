#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_trace_in_line.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_camera_trace_in_line(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_anim_camera_trace_in_line_t trace_in_line = ui_sprite_anim_camera_trace_in_line_create(fsm_state, name);

	return ui_sprite_fsm_action_from_data(trace_in_line);
}

