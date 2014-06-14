#include <assert.h>
#include "ui_data_action_i.h"

void ui_data_action_use_remove_frame(ui_data_actor_frame_t actor_frame) {
    ui_data_action_t action = actor_frame->m_layer->m_actor->m_action;
    ui_data_action_use_t use = actor_frame->m_use;

    assert(use);

    TAILQ_REMOVE(&use->m_user_frames, actor_frame, m_next_for_use);

    if (TAILQ_EMPTY(&use->m_user_frames)) {
        ui_data_src_ref_free(use->m_use);
        TAILQ_REMOVE(&action->m_uses, use, m_next_for_action);
        mem_free(action->m_mgr->m_alloc, use);
    }

    actor_frame->m_use = NULL;
}

int ui_data_action_use_add_frame(ui_data_actor_frame_t actor_frame) {
    ui_data_action_t action = actor_frame->m_layer->m_actor->m_action;
    ui_data_action_use_t use;
    uint32_t ref_src_id;
    ui_data_src_type_t using_src_type;

    switch(actor_frame->m_data.texture.type) {
    case UI_TEXTURE_REF_IMG:
        ref_src_id = actor_frame->m_data.texture.data.img.module_id;
        using_src_type = ui_data_src_type_module;
        break;
    case UI_TEXTURE_REF_FRAME:
        ref_src_id = actor_frame->m_data.texture.data.frame.sprite_id;
        using_src_type = ui_data_src_type_sprite;
        break;
    default:
        CPE_ERROR(action->m_mgr->m_em, "unknown texture ref type %d", actor_frame->m_data.texture.type);
        return -1;
    }

    assert(actor_frame->m_use == NULL);

    TAILQ_FOREACH(use, &action->m_uses, m_next_for_action) {
        if (use->m_use->m_using_src_id == ref_src_id) break;
    }

    if (use == NULL) {
        use = mem_alloc(action->m_mgr->m_alloc, sizeof(struct ui_data_action_use));
        if (use == NULL) {
            CPE_ERROR(action->m_mgr->m_em, "create action_use: alloc fail!");
            return -1;
        }

        use->m_use = ui_data_src_ref_create(ref_src_id, using_src_type, NULL, action->m_src);
        if (use->m_use == NULL) {
            CPE_ERROR(action->m_mgr->m_em, "create action_use: create src_ref fail!");
            mem_free(action->m_mgr->m_alloc, use);
            return -1;
        }

        TAILQ_INSERT_TAIL(&action->m_uses, use, m_next_for_action);

        TAILQ_INIT(&use->m_user_frames);
    }

    TAILQ_INSERT_TAIL(&use->m_user_frames, actor_frame, m_next_for_use);
    actor_frame->m_use = use;

    return 0;
}

static ui_data_src_ref_t ui_data_action_use_next_ref(ui_data_src_ref_it_t it) {
    ui_data_action_use_t * data = (ui_data_action_use_t *)(it->m_data);
    ui_data_action_use_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_action);

    return r->m_use;
}

void ui_data_action_use_refs(ui_data_src_ref_it_t it, ui_data_action_t action) {
    *(ui_data_action_use_t *)(it->m_data) = TAILQ_FIRST(&action->m_uses);
    it->next = ui_data_action_use_next_ref;
}

static ui_data_src_t ui_data_action_use_next_src(ui_data_src_it_t it) {
    ui_data_action_use_t * data = (ui_data_action_use_t *)(it->m_data);
    ui_data_action_use_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_action);

    return r->m_use->m_using_src;
}

void ui_data_action_use_srcs(ui_data_src_it_t it, ui_data_action_t action) {
    *(ui_data_action_use_t *)(it->m_data) = TAILQ_FIRST(&action->m_uses);
    it->next = ui_data_action_use_next_src;
}
