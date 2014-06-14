#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/model/ui_data_src.h"
#include "ui_data_sprite_i.h"
#include "ui_data_src_i.h"

ui_data_frame_t ui_data_frame_create(ui_data_sprite_t sprite, uint32_t id) {
    ui_data_mgr_t mgr = sprite->m_mgr;
    ui_data_frame_t frame;

    frame = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_frame));
    if (frame == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in sprite %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, sprite->m_src));
        return NULL;
    }

    frame->m_sprite = sprite;
    bzero(&frame->m_data, sizeof(frame->m_data));
    frame->m_data.id = id;

    TAILQ_INIT(&frame->m_img_refs);

    TAILQ_INSERT_TAIL(&sprite->m_frames, frame, m_next_for_sprite);

    return frame;
}

void ui_data_frame_free(ui_data_frame_t frame) {
    ui_data_sprite_t sprite = frame->m_sprite;
    ui_data_mgr_t mgr = sprite->m_mgr;

    while(!TAILQ_EMPTY(&frame->m_img_refs)) {
        ui_data_frame_img_free(TAILQ_FIRST(&frame->m_img_refs));
    }

    TAILQ_REMOVE(&sprite->m_frames, frame, m_next_for_sprite);

    mem_free(mgr->m_alloc, frame);
}

static ui_data_frame_t ui_data_frame_in_sprite_next(ui_data_frame_it_t it) {
    ui_data_frame_t * data = (ui_data_frame_t *)(it->m_data);
    ui_data_frame_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_sprite);

    return r;
}

void ui_data_sprite_frames(ui_data_frame_it_t it, ui_data_sprite_t sprite) {
    *(ui_data_frame_t *)(it->m_data) = TAILQ_FIRST(&sprite->m_frames);
    it->next = ui_data_frame_in_sprite_next;
}

UI_FRAME * ui_data_frame_data(ui_data_frame_t frame) {
    return &frame->m_data;
}

LPDRMETA ui_data_frame_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_frame;
}

uint32_t ui_data_frame_hash(const ui_data_frame_t frame) {
    return frame->m_sprite->m_src->m_id & frame->m_data.id;
}

int ui_data_frame_eq(const ui_data_frame_t l, const ui_data_frame_t r) {
    return l->m_data.id == r->m_data.id && l->m_sprite == r->m_sprite;
}
