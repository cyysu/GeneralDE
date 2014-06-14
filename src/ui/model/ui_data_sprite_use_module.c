#include <assert.h>
#include "ui_data_sprite_i.h"

void ui_data_sprite_use_remove_img(ui_data_frame_img_t img_ref) {
    ui_data_sprite_t sprite = img_ref->m_frame->m_sprite;
    ui_data_sprite_use_t sprite_use = img_ref->m_sprite_use;

    assert(sprite_use);
    assert(sprite_use->m_src_ref);

    TAILQ_REMOVE(&sprite_use->m_user_img_refs, img_ref, m_next_for_use);

    if (TAILQ_EMPTY(&sprite_use->m_user_img_refs)) {
        ui_data_src_ref_free(sprite_use->m_src_ref);
        TAILQ_REMOVE(&sprite->m_uses, sprite_use, m_next_for_sprite);
        mem_free(sprite->m_mgr->m_alloc, sprite_use);
    }

    img_ref->m_sprite_use = NULL;
}

int ui_data_sprite_use_add_img(ui_data_frame_img_t img_ref) {
    ui_data_sprite_t sprite = img_ref->m_frame->m_sprite;
    ui_data_sprite_use_t sprite_use;

    assert(img_ref->m_sprite_use == NULL);

    TAILQ_FOREACH(sprite_use, &sprite->m_uses, m_next_for_sprite) {
        assert(sprite_use->m_src_ref);
        if (sprite_use->m_src_ref->m_using_src_id == img_ref->m_data.module_id) break;
    }

    if (sprite_use == NULL) {
        sprite_use = mem_alloc(sprite->m_mgr->m_alloc, sizeof(struct ui_data_sprite_use));
        if (sprite_use == NULL) {
            CPE_ERROR(sprite->m_mgr->m_em, "create sprite_use: alloc fail!");
            return -1;
        }

        sprite_use->m_src_ref = ui_data_src_ref_create(img_ref->m_data.module_id, ui_data_src_type_module, NULL, sprite->m_src);
        if (sprite_use->m_src_ref == NULL) {
            CPE_ERROR(sprite->m_mgr->m_em, "create sprite_use: create src_ref fail!");
            mem_free(sprite->m_mgr->m_alloc, sprite_use);
            return -1;
        }

        TAILQ_INSERT_TAIL(&sprite->m_uses, sprite_use, m_next_for_sprite);

        TAILQ_INIT(&sprite_use->m_user_img_refs);
    }

    TAILQ_INSERT_TAIL(&sprite_use->m_user_img_refs, img_ref, m_next_for_use);
    img_ref->m_sprite_use = sprite_use;

    assert(sprite_use);
    assert(sprite_use->m_src_ref);

    return 0;
}

static ui_data_src_ref_t ui_data_sprite_use_next_ref(ui_data_src_ref_it_t it) {
    ui_data_sprite_use_t * data = (ui_data_sprite_use_t *)(it->m_data);
    ui_data_sprite_use_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_sprite);

    return r->m_src_ref;
}

void ui_data_sprite_use_refs(ui_data_src_ref_it_t it, ui_data_sprite_t sprite) {
    *(ui_data_sprite_use_t *)(it->m_data) = TAILQ_FIRST(&sprite->m_uses);
    it->next = ui_data_sprite_use_next_ref;
}

static ui_data_src_t ui_data_sprite_use_next_src(ui_data_src_it_t it) {
    ui_data_sprite_use_t * data = (ui_data_sprite_use_t *)(it->m_data);
    ui_data_sprite_use_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_sprite);

    return r->m_src_ref->m_using_src;
}

void ui_data_sprite_use_srcs(ui_data_src_it_t it, ui_data_sprite_t sprite) {
    *(ui_data_sprite_use_t *)(it->m_data) = TAILQ_FIRST(&sprite->m_uses);
    it->next = ui_data_sprite_use_next_src;
}
