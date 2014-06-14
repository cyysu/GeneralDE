#include <assert.h>
#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_touch_click_i.h"
#include "ui_sprite_touch_touchable_i.h"

ui_sprite_touch_click_t ui_sprite_touch_click_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_TOUCH_CLICK_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_touch_click_free(ui_sprite_touch_click_t click) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(click);
    ui_sprite_fsm_action_free(fsm_action);
}

uint8_t ui_sprite_touch_click_is_grab(ui_sprite_touch_click_t click) {
    return click->m_responser.m_is_grab;
}

int ui_sprite_touch_click_set_is_grab(ui_sprite_touch_click_t click, uint8_t is_grab) {
    return ui_sprite_touch_responser_set_is_grab(&click->m_responser, is_grab);
}

uint8_t ui_sprite_touch_click_finger_count(ui_sprite_touch_click_t click) {
    return click->m_responser.m_finger_count;
}

int ui_sprite_touch_click_set_finger_count(ui_sprite_touch_click_t click, uint8_t finger_count) {
    return ui_sprite_touch_responser_set_finger_count(&click->m_responser, finger_count);
}

const char * ui_sprite_touch_click_on_click_down(ui_sprite_touch_click_t click) {
    return click->m_on_click_down;
}

int ui_sprite_touch_click_set_on_click_down(ui_sprite_touch_click_t click, const char * on_click_down) {
    ui_sprite_touch_mgr_t mgr = click->m_responser.m_touchable->m_mgr;

    if (click->m_on_click_down) mem_free(mgr->m_alloc, click->m_on_click_down);

    if (on_click_down) {
        click->m_on_click_down = cpe_str_mem_dup(mgr->m_alloc, on_click_down);
        return click->m_on_click_down == NULL ? -1 : 0;
    }
    else {
        click->m_on_click_down = NULL;
        return 0;
    }
}

const char * ui_sprite_touch_click_on_click_up(ui_sprite_touch_click_t click) {
    return click->m_on_click_up;
}

int ui_sprite_touch_click_set_on_click_up(ui_sprite_touch_click_t click, const char * on_click_up) {
    ui_sprite_touch_mgr_t mgr = click->m_responser.m_touchable->m_mgr;

    if (click->m_on_click_up) mem_free(mgr->m_alloc, click->m_on_click_up);

    if (on_click_up) {
        click->m_on_click_up = cpe_str_mem_dup(mgr->m_alloc, on_click_up);
        return click->m_on_click_up == NULL ? -1 : 0;
    }
    else {
        click->m_on_click_up = NULL;
        return 0;
    }
}

static void ui_sprite_touch_click_process_begin(void * ctx) {
    ui_sprite_touch_click_t click = ctx;

    if (click->m_on_click_down) {
        ui_sprite_fsm_action_build_and_send_event(ui_sprite_fsm_action_from_data(click), click->m_on_click_down, NULL);
    }
}

static void ui_sprite_touch_click_process_end(void * ctx) {
    ui_sprite_touch_click_t click = ctx;

    if (click->m_on_click_up) {
        ui_sprite_fsm_action_build_and_send_event(ui_sprite_fsm_action_from_data(click), click->m_on_click_up, NULL);
    }
}

static int ui_sprite_touch_click_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_click_t click = ui_sprite_fsm_action_data(fsm_action);

    return ui_sprite_touch_responser_enter(&click->m_responser);
}

static void ui_sprite_touch_click_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_click_t click = ui_sprite_fsm_action_data(fsm_action);

    ui_sprite_touch_responser_exit(&click->m_responser);
}

static int ui_sprite_touch_click_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_click_t click = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_touch_touchable_t touchable = ui_sprite_touch_touchable_find(entity);

    if (touchable == NULL) {
        CPE_ERROR(
            mgr->m_em, "entity %d(%s): create %s: touchable not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), UI_SPRITE_TOUCH_CLICK_NAME);
        return -1;
    }

    bzero(click, sizeof(struct ui_sprite_touch_click));

    if (ui_sprite_touch_responser_init(&click->m_responser, entity, touchable) != 0) {
        return -1;
    }

    click->m_responser.m_finger_count = 1;
    click->m_responser.m_threshold = 0;
    click->m_responser.m_on_begin = ui_sprite_touch_click_process_begin;
    click->m_responser.m_on_move = NULL;
    click->m_responser.m_on_end = ui_sprite_touch_click_process_end;
    click->m_responser.m_on_cancel = NULL;

    return 0;
}

static void ui_sprite_touch_click_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_click_t click = ui_sprite_fsm_action_data(fsm_action);

    ui_sprite_touch_responser_fini(&click->m_responser);

    if (click->m_on_click_down) mem_free(mgr->m_alloc, click->m_on_click_down);
    if (click->m_on_click_up) mem_free(mgr->m_alloc, click->m_on_click_up);
}

static int ui_sprite_touch_click_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_click_t to_click = ui_sprite_fsm_action_data(to);
    ui_sprite_touch_click_t from_click = ui_sprite_fsm_action_data(from);

    if (ui_sprite_touch_responser_init(
            &to_click->m_responser,
            ui_sprite_fsm_action_to_entity(to),
            ui_sprite_fsm_action_data(to))
        != 0) 
    {
        return -1;
    }

    to_click->m_responser.m_finger_count = from_click->m_responser.m_finger_count;
    to_click->m_on_click_down = from_click->m_on_click_down ? cpe_str_mem_dup(mgr->m_alloc, from_click->m_on_click_down) : NULL;
    to_click->m_on_click_up = from_click->m_on_click_up ? cpe_str_mem_dup(mgr->m_alloc, from_click->m_on_click_up) : NULL;

    return -1;
}

int ui_sprite_touch_click_regist(ui_sprite_touch_mgr_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm, UI_SPRITE_TOUCH_CLICK_NAME, sizeof(struct ui_sprite_touch_click));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: touch click register: meta create fail",
            ui_sprite_touch_mgr_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_touch_click_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_touch_click_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_touch_click_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_touch_click_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_touch_click_clear, module);

    return 0;
}

void ui_sprite_touch_click_unregist(ui_sprite_touch_mgr_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm, UI_SPRITE_TOUCH_CLICK_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: touch click unregister: meta not exist",
            ui_sprite_touch_mgr_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_TOUCH_CLICK_NAME = "touch-click";
