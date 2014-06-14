#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui_data_action_i.h"

ui_data_actor_frame_t ui_data_actor_frame_create(ui_data_actor_layer_t actor_layer) {
    ui_data_mgr_t mgr = actor_layer->m_actor->m_action->m_mgr;
    ui_data_actor_frame_t actor_frame;

    actor_frame = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_actor_frame));
    if (actor_frame == NULL) {
        CPE_ERROR(mgr->m_em, "create actor_frame fail !");
        return NULL;
    }

    actor_frame->m_layer = actor_layer;
    actor_frame->m_use = NULL;
    bzero(&actor_frame->m_data, sizeof(actor_frame->m_data));

    TAILQ_INSERT_TAIL(&actor_layer->m_frames, actor_frame, m_next_for_layer);

    return actor_frame;
}

void ui_data_actor_frame_free(ui_data_actor_frame_t actor_frame) {
    ui_data_actor_layer_t actor_layer = actor_frame->m_layer;
    ui_data_mgr_t mgr = actor_layer->m_actor->m_action->m_mgr;

    if (actor_frame->m_use) {
        ui_data_action_use_remove_frame(actor_frame);
        assert(actor_frame->m_use == NULL);
    }

    TAILQ_REMOVE(&actor_layer->m_frames, actor_frame, m_next_for_layer);

    mem_free(mgr->m_alloc, actor_frame);
}

static ui_data_actor_frame_t ui_data_actor_frame_in_layer_next(ui_data_actor_frame_it_t it) {
    ui_data_actor_frame_t * data = (ui_data_actor_frame_t *)(it->m_data);
    ui_data_actor_frame_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_layer);

    return r;
}

void ui_data_actor_layer_frames(ui_data_actor_frame_it_t it, ui_data_actor_layer_t layer) {
    *(ui_data_actor_frame_t *)(it->m_data) = TAILQ_FIRST(&layer->m_frames);
    it->next = ui_data_actor_frame_in_layer_next;
}

static ui_data_actor_frame_t ui_data_actor_frame_in_actor_layer_next(struct ui_data_actor_frame_it * it) {
    ui_data_actor_frame_t * data = (ui_data_actor_frame_t *)(it->m_data);
    ui_data_actor_frame_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_layer);

    return r;
}

void ui_data_actor_frame_refs(ui_data_actor_frame_it_t it, ui_data_actor_layer_t actor_layer) {
    *(ui_data_actor_frame_t *)(it->m_data) = TAILQ_FIRST(&actor_layer->m_frames);
    it->next = ui_data_actor_frame_in_actor_layer_next;
}

UI_ACTOR_FRAME * ui_data_actor_frame_data(ui_data_actor_frame_t actor_frame) {
    return &actor_frame->m_data;
}

LPDRMETA ui_data_actor_frame_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_actor_frame;
}

int ui_data_actor_frame_update_ref(ui_data_actor_frame_t actor_frame) {
    ui_data_action_t action = actor_frame->m_layer->m_actor->m_action;
    uint32_t ref_src_id;

    switch(actor_frame->m_data.texture.type) {
    case UI_TEXTURE_REF_IMG:
        ref_src_id = actor_frame->m_data.texture.data.img.module_id;
        break;
    case UI_TEXTURE_REF_FRAME:
        ref_src_id = actor_frame->m_data.texture.data.frame.sprite_id;
        break;
    default:
        CPE_ERROR(action->m_mgr->m_em, "unknown texture ref type %d", actor_frame->m_data.texture.type);
        return -1;
    }

    if (actor_frame->m_use) {
        if (actor_frame->m_use->m_use->m_using_src_id == ref_src_id) return 0;
        ui_data_action_use_remove_frame(actor_frame);
    }

    if (ref_src_id == (uint32_t)-1) return 0;

    if (ui_data_action_use_add_frame(actor_frame) != 0) return -1;

    assert(actor_frame->m_use);

    return 0;
}


int ui_data_action_update_refs(ui_data_action_t action) {
    ui_data_actor_t actor;
    ui_data_actor_layer_t actor_layer;
    ui_data_actor_frame_t actor_frame;
    int rv = 0;

    TAILQ_FOREACH(actor, &action->m_actors, m_next_for_action) {
        TAILQ_FOREACH(actor_layer, &actor->m_layers, m_next_for_actor) {
            TAILQ_FOREACH(actor_frame, &actor_layer->m_frames, m_next_for_layer) {
                if (ui_data_actor_frame_update_ref(actor_frame) != 0) rv = -1;
            }
        }
    }

    return rv;
}

