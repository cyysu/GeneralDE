#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_anim/ui_sprite_anim_sch.h"
#include "ui_sprite_anim_show_animation_i.h"
#include "ui_sprite_anim_module_i.h"

ui_sprite_anim_show_animation_t
ui_sprite_anim_show_animation_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_SHOW_ANIMATION_TYPE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_show_animation_free(ui_sprite_anim_show_animation_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

int ui_sprite_anim_show_animation_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_show_animation_t show_animation = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_anim_sch_t anim_sch = ui_sprite_anim_sch_find(entity);

    if (anim_sch == NULL) {
        CPE_ERROR(
            show_animation->m_module->m_em, "entity %d(%s): no anim_sch, can`t start anim %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_res);
        return -1;
    }

    if (show_animation->m_anim_id != UI_SPRITE_INVALID_ANIM_ID) {
        CPE_ERROR(
            show_animation->m_module->m_em, "entity %d(%s): show animation enter: already started!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    show_animation->m_anim_id = ui_sprite_anim_sch_start_anim(
            anim_sch, show_animation->m_group, show_animation->m_res,
            show_animation->m_loop, show_animation->m_start, show_animation->m_end);
    if (show_animation->m_anim_id == UI_SPRITE_INVALID_ANIM_ID) {
        CPE_ERROR(
            show_animation->m_module->m_em, "entity %d(%s): start anim %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_res);
        return -1;
    }

    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    return 0;
}

void ui_sprite_anim_show_animation_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_show_animation_t show_animation = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_anim_sch_t anim_sch = ui_sprite_anim_sch_find(entity);

    if (anim_sch == NULL) {
        CPE_ERROR(
            show_animation->m_module->m_em, "entity %d(%s): no anim_sch, can`t stop anim %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_res);
        return;
    }

    if (show_animation->m_anim_id != UI_SPRITE_INVALID_ANIM_ID) {
        ui_sprite_anim_sch_stop_anim(anim_sch, show_animation->m_anim_id);
        show_animation->m_anim_id = UI_SPRITE_INVALID_ANIM_ID;
    }
}

void ui_sprite_anim_show_animation_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_anim_show_animation_t show_animation = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_anim_sch_t anim_sch = ui_sprite_anim_sch_find(entity);

    if (anim_sch == NULL) {
        CPE_ERROR(
            show_animation->m_module->m_em, "entity %d(%s): no anim_sch, anim %s stop update!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_res);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (!ui_sprite_anim_sch_is_anim_runing(anim_sch, show_animation->m_anim_id)) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                show_animation->m_module->m_em, "entity %d(%s): anim %s stoped",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_res);
        }
        
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
}

int ui_sprite_anim_show_animation_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_show_animation_t show_animation = ui_sprite_fsm_action_data(fsm_action);

    bzero(show_animation, sizeof(*show_animation));

    show_animation->m_module = ctx;
    show_animation->m_anim_id = UI_SPRITE_INVALID_ANIM_ID;

    return 0;
}

int ui_sprite_anim_show_animation_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_anim_show_animation_t to_show_animation = ui_sprite_fsm_action_data(to);

    memcpy(to_show_animation, ui_sprite_fsm_action_data(from), sizeof(*to_show_animation));

    to_show_animation->m_module = ctx;
    to_show_animation->m_anim_id = UI_SPRITE_INVALID_ANIM_ID;

    return 0;
}

void ui_sprite_anim_show_animation_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_anim_show_animation_regist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_ANIM_SHOW_ANIMATION_TYPE_NAME, sizeof(struct ui_sprite_anim_show_animation));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_anim_module_name(module), UI_SPRITE_ANIM_SHOW_ANIMATION_TYPE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_anim_show_animation_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_anim_show_animation_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_anim_show_animation_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_anim_show_animation_exit, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_anim_show_animation_update, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_anim_show_animation_clear, module);

    return 0;
}

void ui_sprite_anim_show_animation_unregist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_ANIM_SHOW_ANIMATION_TYPE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_anim_module_name(module), UI_SPRITE_ANIM_SHOW_ANIMATION_TYPE_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * ui_sprite_anim_show_animation_group(ui_sprite_anim_show_animation_t show_animation) {
    return show_animation->m_group;
}

void ui_sprite_anim_show_animation_set_group(ui_sprite_anim_show_animation_t show_animation, const char * group) {
    strncpy(show_animation->m_group, group, sizeof(show_animation->m_group));
}

const char * ui_sprite_anim_show_animation_res(ui_sprite_anim_show_animation_t show_animation) {
    return show_animation->m_res;
}

void ui_sprite_anim_show_animation_set_res(ui_sprite_anim_show_animation_t show_animation, const char * res) {
    strncpy(show_animation->m_res, res, sizeof(show_animation->m_res));
}

uint8_t ui_sprite_anim_show_animation_loop(ui_sprite_anim_show_animation_t show_animation) {
    return show_animation->m_loop;
}

void ui_sprite_anim_show_animation_set_loop(ui_sprite_anim_show_animation_t show_animation, uint8_t loop) {
    show_animation->m_loop = loop;
}

int32_t ui_sprite_anim_show_animation_start(ui_sprite_anim_show_animation_t show_animation) {
    return show_animation->m_start;
}

void ui_sprite_anim_show_animation_set_start(ui_sprite_anim_show_animation_t show_animation, int32_t start) {
    show_animation->m_start = start;
}

int32_t ui_sprite_anim_show_animation_end(ui_sprite_anim_show_animation_t show_animation) {
    return show_animation->m_end;
}

void ui_sprite_anim_show_animation_set_end(ui_sprite_anim_show_animation_t show_animation, int32_t end) {
    show_animation->m_end = end;
}

const char * UI_SPRITE_ANIM_SHOW_ANIMATION_TYPE_NAME = "show-animation";
