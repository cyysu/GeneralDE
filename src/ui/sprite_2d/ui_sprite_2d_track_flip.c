#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_track_flip.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_track_flip_i.h"
#include "ui_sprite_2d_module_i.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_evt.h"

ui_sprite_2d_track_flip_t
ui_sprite_2d_track_flip_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_TRACK_FLIP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_track_flip_free(ui_sprite_2d_track_flip_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

uint8_t ui_sprite_2d_track_flip_pos(ui_sprite_2d_track_flip_t track_flip) {
    return track_flip->m_track_pos;
}

void ui_sprite_2d_track_flip_set_pos(ui_sprite_2d_track_flip_t track_flip, uint8_t pos_policy) {
    track_flip->m_track_pos = pos_policy;
}

uint8_t ui_sprite_2d_track_flip_process_x(ui_sprite_2d_track_flip_t track_flip) {
    return track_flip->m_process_x;
}

void ui_sprite_2d_track_flip_set_process_x(ui_sprite_2d_track_flip_t track_flip, uint8_t process_x) {
    track_flip->m_process_x = process_x;
}

uint8_t ui_sprite_2d_track_flip_process_y(ui_sprite_2d_track_flip_t track_flip) {
    return track_flip->m_process_y;
}

void ui_sprite_2d_track_flip_set_process_y(ui_sprite_2d_track_flip_t track_flip, uint8_t process_y) {
    track_flip->m_process_y = process_y;
}

int ui_sprite_2d_track_flip_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_track_flip_t track_flip = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track flip: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_fsm_action_start_update(fsm_action);

    track_flip->m_pre_pos = ui_sprite_2d_transform_pos(transform, track_flip->m_track_pos);

    return 0;
}

void ui_sprite_2d_track_flip_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_track_flip_t track_flip = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    UI_SPRITE_2D_PAIR cur_pos;

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track flip: update: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    cur_pos = ui_sprite_2d_transform_pos(transform, track_flip->m_track_pos);

    if (track_flip->m_process_x && fabs(cur_pos.x - track_flip->m_pre_pos.x) > 0.5f) {
        ui_sprite_2d_transform_set_flip_x(transform, cur_pos.x > track_flip->m_pre_pos.x ? 0 : 1);
    }

    if (track_flip->m_process_y && fabs(cur_pos.y - track_flip->m_pre_pos.y) > 0.5f) {
        ui_sprite_2d_transform_set_flip_y(transform, cur_pos.y < track_flip->m_pre_pos.y ? 0 : 1);
    }

    track_flip->m_pre_pos = cur_pos;
}

void ui_sprite_2d_track_flip_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_track_flip_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_track_flip_t track_flip = ui_sprite_fsm_action_data(fsm_action);

	bzero(track_flip, sizeof(*track_flip));
	track_flip->m_module = ctx;
    track_flip->m_track_pos = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;

	return 0;
}

int ui_sprite_2d_track_flip_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_track_flip_t to_track_flip_to = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_track_flip_t from_track_flip_to = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_track_flip_init(to, ctx);

    to_track_flip_to->m_process_x = from_track_flip_to->m_process_x;
    to_track_flip_to->m_process_y = from_track_flip_to->m_process_y;
    to_track_flip_to->m_track_pos = from_track_flip_to->m_track_pos;

    return 0;
}

void ui_sprite_2d_track_flip_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_track_flip_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_TRACK_FLIP_NAME, sizeof(struct ui_sprite_2d_track_flip));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRACK_FLIP_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_track_flip_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_track_flip_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_track_flip_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_track_flip_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_track_flip_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_track_flip_update, module);

    return 0;
}

void ui_sprite_2d_track_flip_unregist(ui_sprite_2d_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_TRACK_FLIP_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRACK_FLIP_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_2D_TRACK_FLIP_NAME = "2d-track-flip";
