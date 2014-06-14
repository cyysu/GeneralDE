#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_anim/ui_sprite_anim_sch.h"
#include "ui_sprite_touch_touchable_i.h"
#include "ui_sprite_touch_responser_i.h"
#include "ui_sprite_touch_box_i.h"
#include "ui_sprite_touch_mgr_i.h"

ui_sprite_touch_touchable_t ui_sprite_touch_touchable_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_TOUCH_TOUCHABLE_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

ui_sprite_touch_touchable_t ui_sprite_touch_touchable_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_TOUCH_TOUCHABLE_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

void ui_sprite_touch_touchable_free(ui_sprite_touch_touchable_t touch_touchable) {
    ui_sprite_component_t component = ui_sprite_component_from_data(touch_touchable);
    ui_sprite_component_free(component);
};

int ui_sprite_touch_touchable_is_point_in(ui_sprite_touch_touchable_t touchable, UI_SPRITE_2D_PAIR world_pt) {
    ui_sprite_touch_mgr_t mgr = touchable->m_mgr;
    ui_sprite_entity_t entity;
    ui_sprite_2d_transform_t transform;
    ui_sprite_touch_box_t box;

    if (TAILQ_EMPTY(&touchable->m_boxes)) return 1;

    entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));

    if ((transform = ui_sprite_2d_transform_find(entity))) {
        UI_SPRITE_2D_PAIR sprite_pos = ui_sprite_2d_transform_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN);
        world_pt.x -= sprite_pos.x;
        world_pt.y -= sprite_pos.y;
    }

    TAILQ_FOREACH(box, &touchable->m_boxes, m_next_for_touchable) {
        if (world_pt.x >= box->m_lt.x && world_pt.x <= box->m_rb.x
            && world_pt.y >= box->m_lt.y && world_pt.y <= box->m_rb.y)
        {
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    mgr->m_em, "entity %d(%s): touch check success",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            }
            return 1;
        }
    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            mgr->m_em, "entity %d(%s): touch check fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
    }

    return 0;
}

static void ui_sprite_touch_show_boxes(ui_sprite_touch_touchable_t touch) {
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(touch));
    ui_sprite_touch_box_t box;
    ui_sprite_anim_sch_t anim = ui_sprite_anim_sch_find(entity);

    if (anim == NULL) {
        CPE_ERROR(
            touch->m_mgr->m_em, "%d(%s): touchable: show boxes: no anim!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    TAILQ_FOREACH(box, &touch->m_boxes, m_next_for_touchable) {
        char buf[128];
        snprintf(buf, sizeof(buf), "BOX: lt.x=%f, lt.y=%f, rb.x=%f, rb.y=%f, color=1", box->m_lt.x, box->m_lt.y, box->m_rb.x, box->m_rb.y);

        assert(box->m_box_id == UI_SPRITE_INVALID_ANIM_ID);
        box->m_box_id = ui_sprite_anim_sch_start_anim(anim, "", buf, 0, -1, -1);
        if (box->m_box_id == UI_SPRITE_INVALID_ANIM_ID) {
            CPE_ERROR(
                touch->m_mgr->m_em, "%d(%s): touchable: show boxes: start '%s' fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), buf);
        }
    }
}

static void ui_sprite_touch_hide_boxes(ui_sprite_touch_touchable_t touch) {
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(touch));
    ui_sprite_touch_box_t box;
    ui_sprite_anim_sch_t anim = ui_sprite_anim_sch_find(entity);

    if (anim == NULL) return;

    TAILQ_FOREACH(box, &touch->m_boxes, m_next_for_touchable) {
        if (box->m_box_id != UI_SPRITE_INVALID_ANIM_ID) {
            ui_sprite_anim_sch_stop_anim(anim, box->m_box_id);
        }
        box->m_box_id = UI_SPRITE_INVALID_ANIM_ID;
    }
}


static int ui_sprite_touch_touchable_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_touch_touchable_t touch = ui_sprite_component_data(component);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);

    if (ui_sprite_entity_debug(entity)) {
        ui_sprite_touch_show_boxes(touch);
    }

    return 0;
}

static void ui_sprite_touch_touchable_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_touch_touchable_t touch = ui_sprite_component_data(component);

    ui_sprite_touch_hide_boxes(touch);
}

static int ui_sprite_touch_touchable_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_touch_touchable_t touch = ui_sprite_component_data(component);

    touch->m_mgr = ctx;
    TAILQ_INIT(&touch->m_responsers);
    TAILQ_INIT(&touch->m_boxes);

    return 0;
}

static void ui_sprite_touch_touchable_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_touch_touchable_t touch = ui_sprite_component_data(component);

    while(!TAILQ_EMPTY(&touch->m_boxes)) {
        ui_sprite_touch_box_free(TAILQ_FIRST(&touch->m_boxes));
    }

    assert(TAILQ_EMPTY(&touch->m_responsers));
}

static int ui_sprite_touch_touchable_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_touchable_t to_touch = ui_sprite_component_data(to);
    ui_sprite_entity_t to_entity = ui_sprite_component_entity(to);
    ui_sprite_touch_touchable_t from_touch = ui_sprite_component_data(from);
    ui_sprite_touch_box_t from_box;

    if (ui_sprite_touch_touchable_init(to, ctx) != 0) return -1;

    TAILQ_FOREACH(from_box, &from_touch->m_boxes, m_next_for_touchable) {
        ui_sprite_touch_box_t to_box =
            ui_sprite_touch_box_create(to_touch, from_box->m_lt, from_box->m_rb);
        if (to_box == NULL) {
            CPE_ERROR(
                mgr->m_em, "entity %d(%s): Touchable: copy: box create fail!",
                ui_sprite_entity_id(to_entity), ui_sprite_entity_name(to_entity));
            ui_sprite_touch_touchable_fini(to, ctx);
            return -1;
        }
    }

    return 0;
}

int ui_sprite_touch_touchable_regist(ui_sprite_touch_mgr_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(module->m_repo, UI_SPRITE_TOUCH_TOUCHABLE_NAME, sizeof(struct ui_sprite_touch_touchable));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: touch component register: meta create fail",
            ui_sprite_touch_mgr_name(module));
        return -1;
    }

    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_touch_touchable_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_touch_touchable_exit, module);
    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_touch_touchable_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_touch_touchable_copy, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_touch_touchable_fini, module);

    return 0;
}

void ui_sprite_touch_touchable_unregist(ui_sprite_touch_mgr_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_TOUCH_TOUCHABLE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: touch component unregister: meta not exist",
            ui_sprite_touch_mgr_name(module));
        return;
    }

    ui_sprite_component_meta_free(meta);
}

const char * UI_SPRITE_TOUCH_TOUCHABLE_NAME = "Touchable";
