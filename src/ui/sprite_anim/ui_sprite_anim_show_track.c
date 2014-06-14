#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_anim/ui_sprite_anim_show_track.h"
#include "ui_sprite_anim_show_track_i.h"
#include "ui_sprite_anim_module_i.h"
#include "protocol/ui/sprite_anim/ui_sprite_anim_evt.h"

ui_sprite_anim_show_track_t
ui_sprite_anim_show_track_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_SHOW_TRACK_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_show_track_free(ui_sprite_anim_show_track_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

uint8_t ui_sprite_anim_show_track_pos(ui_sprite_anim_show_track_t show_track) {
    return show_track->m_track_pos;
}

void ui_sprite_anim_show_track_set_pos(ui_sprite_anim_show_track_t show_track, uint8_t pos_policy) {
    show_track->m_track_pos = pos_policy;
}

int ui_sprite_anim_show_track_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_module_t module = ctx;
	ui_sprite_anim_show_track_t show_track = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track flip: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_fsm_action_start_update(fsm_action);

    show_track->m_pre_pos = ui_sprite_2d_transform_pos(transform, show_track->m_track_pos);

    return 0;
}

void ui_sprite_anim_show_track_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_anim_module_t module = ctx;
	ui_sprite_anim_show_track_t show_track = ui_sprite_fsm_action_data(fsm_action);
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

    cur_pos = ui_sprite_2d_transform_pos(transform, show_track->m_track_pos);
}

void ui_sprite_anim_show_track_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_anim_show_track_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_anim_show_track_t show_track = ui_sprite_fsm_action_data(fsm_action);

	bzero(show_track, sizeof(*show_track));
	show_track->m_module = ctx;
    show_track->m_track_pos = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;

	return 0;
}

int ui_sprite_anim_show_track_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_anim_show_track_t to_show_track_to = ui_sprite_fsm_action_data(to);
    ui_sprite_anim_show_track_t from_show_track_to = ui_sprite_fsm_action_data(from);

    ui_sprite_anim_show_track_init(to, ctx);

    to_show_track_to->m_track_pos = from_show_track_to->m_track_pos;

    return 0;
}

void ui_sprite_anim_show_track_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_anim_show_track_regist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_ANIM_SHOW_TRACK_NAME, sizeof(struct ui_sprite_anim_show_track));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_anim_module_name(module), UI_SPRITE_ANIM_SHOW_TRACK_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_anim_show_track_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_anim_show_track_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_anim_show_track_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_anim_show_track_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_anim_show_track_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_anim_show_track_update, module);

    return 0;
}

void ui_sprite_anim_show_track_unregist(ui_sprite_anim_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_ANIM_SHOW_TRACK_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_anim_module_name(module), UI_SPRITE_ANIM_SHOW_TRACK_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_ANIM_SHOW_TRACK_NAME = "show-track";
