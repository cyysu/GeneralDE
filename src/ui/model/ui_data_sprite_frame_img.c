#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/model/ui_data_src.h"
#include "ui_data_sprite_i.h"
#include "ui_data_src_i.h"

ui_data_frame_img_t ui_data_frame_img_create(ui_data_frame_t frame) {
    ui_data_mgr_t mgr = frame->m_sprite->m_mgr;
    ui_data_frame_img_t img_ref;

    img_ref = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_frame_img));
    if (img_ref == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in frame %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, frame->m_sprite->m_src));
        return NULL;
    }

    img_ref->m_frame = frame;
    img_ref->m_sprite_use = NULL;
    bzero(&img_ref->m_data, sizeof(img_ref->m_data));

    TAILQ_INSERT_TAIL(&frame->m_img_refs, img_ref, m_next_for_frame);

    return img_ref;
}

void ui_data_frame_img_free(ui_data_frame_img_t img_ref) {
    ui_data_frame_t frame = img_ref->m_frame;
    ui_data_mgr_t mgr = frame->m_sprite->m_mgr;

    if (img_ref->m_sprite_use) {
        ui_data_sprite_use_remove_img(img_ref);
        assert(img_ref->m_sprite_use == NULL);
    }

    TAILQ_REMOVE(&frame->m_img_refs, img_ref, m_next_for_frame);

    mem_free(mgr->m_alloc, img_ref);
}

static ui_data_frame_img_t ui_data_frame_img_in_frame_next(struct ui_data_frame_img_it * it) {
    ui_data_frame_img_t * data = (ui_data_frame_img_t *)(it->m_data);
    ui_data_frame_img_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_frame);

    return r;
}

void ui_data_frame_imgs(ui_data_frame_img_it_t it, ui_data_frame_t frame) {
    *(ui_data_frame_img_t *)(it->m_data) = TAILQ_FIRST(&frame->m_img_refs);
    it->next = ui_data_frame_img_in_frame_next;
}

UI_IMG_REF * ui_data_frame_img_data(ui_data_frame_img_t img_ref) {
    return &img_ref->m_data;
}

LPDRMETA ui_data_frame_img_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_frame_img;
}

int ui_data_frame_img_update_ref(ui_data_frame_img_t img_ref) {
    if (img_ref->m_sprite_use) {
        assert(img_ref->m_sprite_use->m_src_ref);

        if (img_ref->m_sprite_use->m_src_ref->m_using_src_id == img_ref->m_data.module_id) return 0;

        ui_data_sprite_use_remove_img(img_ref);
        assert(img_ref->m_sprite_use == NULL);
    }

    if (img_ref->m_data.module_id == 0) return 0;

    if (ui_data_sprite_use_add_img(img_ref) != 0) return -1;

    assert(img_ref->m_sprite_use);

    return 0;
}


int ui_data_sprite_update_refs(ui_data_sprite_t sprite) {
    ui_data_frame_t frame;
    ui_data_frame_img_t img_ref;
    int rv = 0;

    TAILQ_FOREACH(frame, &sprite->m_frames, m_next_for_sprite) {
        TAILQ_FOREACH(img_ref, &frame->m_img_refs, m_next_for_frame) {
            if (ui_data_frame_img_update_ref(img_ref) != 0) rv = -1;
        }
    }

    return rv;
}

