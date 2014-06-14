#include <assert.h>
#include "ui_sprite_touch_box_i.h"
#include "ui_sprite_touch_touchable_i.h"
#include "ui_sprite_touch_mgr_i.h"

ui_sprite_touch_box_t
ui_sprite_touch_box_create(ui_sprite_touch_touchable_t touchable, UI_SPRITE_2D_PAIR lt, UI_SPRITE_2D_PAIR rb) {
    ui_sprite_touch_mgr_t mgr = touchable->m_mgr;
    ui_sprite_touch_box_t box;

    box = mem_alloc(mgr->m_alloc, sizeof(struct ui_sprite_touch_box));
    if (box == NULL) {
        CPE_ERROR(mgr->m_em, "alloc box fail!");
        return NULL;
    }

    box->m_touchable = touchable;
    box->m_lt = lt;
    box->m_rb = rb;
    box->m_box_id = UI_SPRITE_INVALID_ANIM_ID;

    TAILQ_INSERT_TAIL(&touchable->m_boxes, box, m_next_for_touchable);

    return box;
}

void ui_sprite_touch_box_free(ui_sprite_touch_box_t box) {
    ui_sprite_touch_mgr_t mgr = box->m_touchable->m_mgr;
    TAILQ_REMOVE(&box->m_touchable->m_boxes, box, m_next_for_touchable);

    assert(box->m_box_id == UI_SPRITE_INVALID_ANIM_ID);

    mem_free(mgr->m_alloc, box);
}

UI_SPRITE_2D_PAIR ui_sprite_touch_box_lt(ui_sprite_touch_box_t box) {
    return box->m_lt;
}

UI_SPRITE_2D_PAIR ui_sprite_touch_box_rb(ui_sprite_touch_box_t box) {
    return box->m_rb;
}
