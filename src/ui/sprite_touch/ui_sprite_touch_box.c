#include <assert.h>
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui_sprite_touch_box_i.h"
#include "ui_sprite_touch_touchable_i.h"
#include "ui_sprite_touch_mgr_i.h"

ui_sprite_touch_box_t
ui_sprite_touch_box_create(ui_sprite_touch_touchable_t touchable, UI_SPRITE_TOUCH_SHAPE const * shape) {
    ui_sprite_touch_mgr_t mgr = touchable->m_mgr;
    ui_sprite_touch_box_t box;

    box = mem_alloc(mgr->m_alloc, sizeof(struct ui_sprite_touch_box));
    if (box == NULL) {
        CPE_ERROR(mgr->m_em, "alloc box fail!");
        return NULL;
    }

    box->m_touchable = touchable;
    box->m_shape = *shape;
    box->m_box_anim_id = UI_SPRITE_INVALID_ANIM_ID;

    TAILQ_INSERT_TAIL(&touchable->m_boxes, box, m_next_for_touchable);

    return box;
}

void ui_sprite_touch_box_free(ui_sprite_touch_box_t box) {
    ui_sprite_touch_mgr_t mgr = box->m_touchable->m_mgr;
    TAILQ_REMOVE(&box->m_touchable->m_boxes, box, m_next_for_touchable);

    assert(box->m_box_anim_id == UI_SPRITE_INVALID_ANIM_ID);

    mem_free(mgr->m_alloc, box);
}

UI_SPRITE_TOUCH_SHAPE const * ui_sprite_touch_box_shape(ui_sprite_touch_box_t box) {
    return &box->m_shape;
}

uint8_t ui_sprite_touch_box_check_pt_in(
    ui_sprite_touch_box_t box, ui_sprite_entity_t entity, UI_SPRITE_2D_PAIR test_pt)
{
    ui_sprite_touch_mgr_t mgr = box->m_touchable->m_mgr;
    ui_sprite_2d_transform_t transform;

    if ((transform = ui_sprite_2d_transform_find(entity))) {
        test_pt = ui_sprite_2d_transform_world_to_local(transform, test_pt);
    }

    switch(box->m_shape.type) {
    case UI_SPRITE_TOUCH_SHAPE_ENTITY_RECT:
        if (transform == NULL) {
            CPE_ERROR(
                mgr->m_em, "entity %d(%s): touch_box: shape type is entity rect, but no transform!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return 0;
        }
        else {
            UI_SPRITE_2D_RECT rect = *ui_sprite_2d_transform_rect(transform);

            rect.lt.x -= box->m_shape.data.entity_rect.adj.x;
            rect.lt.y -= box->m_shape.data.entity_rect.adj.y;

            rect.rb.x += box->m_shape.data.entity_rect.adj.x;
            rect.rb.y += box->m_shape.data.entity_rect.adj.y;

            return ui_sprite_2d_pt_in_rect(test_pt, &rect);
        }
    case UI_SPRITE_TOUCH_SHAPE_BOX: {
        UI_SPRITE_2D_RECT rect = { { box->m_shape.data.box.lt.x, box->m_shape.data.box.lt.y },
                                   { box->m_shape.data.box.rb.x, box->m_shape.data.box.rb.y } };
        return ui_sprite_2d_pt_in_rect(test_pt, &rect);
    }
    case UI_SPRITE_TOUCH_SHAPE_CIRCLE: {
        UI_SPRITE_2D_PAIR center =
            ui_sprite_2d_transform_local_pos(
                transform, box->m_shape.data.circle.base_pos, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);
        float radius = box->m_shape.data.circle.radius * ui_sprite_2d_transform_scale(transform);
        return ui_sprite_2d_pt_in_circle(test_pt, &center, radius);
    }
    default:
        CPE_ERROR(
            mgr->m_em, "entity %d(%s): touch_box: unknown shape type %d!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), box->m_shape.type);
        return 0;
    }
}
