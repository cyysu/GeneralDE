#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_spine_play_anim_i.h"
#include "ui_sprite_spine_obj_i.h"

ui_sprite_spine_play_anim_t ui_sprite_spine_play_anim_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_PLAY_ANIM_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_play_anim_free(ui_sprite_spine_play_anim_t play_anim) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(play_anim);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_play_anim_set_name(ui_sprite_spine_play_anim_t play_anim, const char * name) {
    
    ui_sprite_spine_module_t module = play_anim->m_module;

    if (play_anim->m_anim_name) mem_free(module->m_alloc, play_anim->m_anim_name);

    if (name) {
        play_anim->m_anim_name = cpe_str_mem_dup(module->m_alloc, name);
        return play_anim->m_anim_name == NULL ? -1 : 0;
    }
    else {
        play_anim->m_anim_name = NULL;
        return 0;
    }
}

const char * ui_sprite_spine_play_anim_name(ui_sprite_spine_play_anim_t play_anim) {
    return play_anim->m_anim_name;
}

void ui_sprite_spine_play_anim_set_is_loop(ui_sprite_spine_play_anim_t play_anim, uint8_t is_loop) {
    play_anim->m_is_loop = is_loop;
}

uint8_t ui_sprite_spine_play_anim_is_loop(ui_sprite_spine_play_anim_t play_anim) {
    return play_anim->m_is_loop;
}

static int ui_sprite_spine_play_anim_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_play_anim_t play_anim = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_spine_obj_t spine_obj;

    spine_obj = ui_sprite_spine_obj_find(entity);
    if (spine_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: not spine obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (play_anim->m_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: anim name not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    assert(spine_obj->m_obj);
    assert(play_anim->m_track_entry == NULL);
    play_anim->m_track_entry = ui_spine_obj_set_animation(spine_obj->m_obj, 0, play_anim->m_anim_name, play_anim->m_is_loop);
    if (play_anim->m_track_entry == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: anim name not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): spine play anim %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), play_anim->m_anim_name);
    }

    return 0;
}

static void ui_sprite_spine_play_anim_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_play_anim_t play_anim = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (play_anim->m_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: exit: anim name not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    assert(play_anim->m_track_entry);
    play_anim->m_track_entry = NULL;

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): spine play anim: exit: stop anim %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), play_anim->m_anim_name);
    }
}

static int ui_sprite_spine_play_anim_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_play_anim_t play_anim = ui_sprite_fsm_action_data(fsm_action);
    play_anim->m_module = ctx;
	play_anim->m_anim_name = NULL;
    play_anim->m_is_loop = 0;
    play_anim->m_track_entry = NULL;
    return 0;
}

static void ui_sprite_spine_play_anim_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_play_anim_t play_anim = ui_sprite_fsm_action_data(fsm_action);

    assert(play_anim->m_track_entry == NULL);

    if (play_anim->m_anim_name) {
        mem_free(module->m_alloc, play_anim->m_anim_name);
        play_anim->m_anim_name = NULL;
    }
}

static int ui_sprite_spine_play_anim_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_play_anim_t to_play_anim = ui_sprite_fsm_action_data(to);
    ui_sprite_spine_play_anim_t from_play_anim = ui_sprite_fsm_action_data(from);

    if (ui_sprite_spine_play_anim_init(to, ctx)) return -1;

    if (from_play_anim->m_anim_name) {
        to_play_anim->m_anim_name = cpe_str_mem_dup(module->m_alloc, from_play_anim->m_anim_name);
    }

    to_play_anim->m_is_loop = from_play_anim->m_is_loop;

    return 0;
}

int ui_sprite_spine_play_anim_regist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_PLAY_ANIM_NAME, sizeof(struct ui_sprite_spine_play_anim));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_spine_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_play_anim_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_play_anim_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_play_anim_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_play_anim_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_play_anim_clear, module);

    return 0;
}

void ui_sprite_spine_play_anim_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_PLAY_ANIM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_SPINE_PLAY_ANIM_NAME = "play-spine-anim";

