#include <assert.h>
#include "ui_data_sprite_i.h"
#include "ui_data_src_i.h"

ui_data_sprite_t ui_data_sprite_create(ui_data_mgr_t mgr, ui_data_src_t src) {
    ui_data_sprite_t sprite;

    if (src->m_type != ui_data_src_type_sprite) {
        CPE_ERROR(
            mgr->m_em, "create sprite at %s: src not sprite!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    if (src->m_product) {
        CPE_ERROR(
            mgr->m_em, "create sprite at %s: product already loaded!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    sprite = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_sprite));
    if (sprite == NULL) {
        CPE_ERROR(
            mgr->m_em, "create sprite at %s: alloc fail!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    sprite->m_mgr = mgr;
    sprite->m_src = src;

    TAILQ_INIT(&sprite->m_frames);
    TAILQ_INIT(&sprite->m_uses);

    src->m_product = sprite;

    return sprite;
}

void ui_data_sprite_free(ui_data_sprite_t sprite) {
    ui_data_mgr_t mgr = sprite->m_mgr;

    while(!TAILQ_EMPTY(&sprite->m_frames)) {
        ui_data_frame_free(TAILQ_FIRST(&sprite->m_frames));
    }

    assert(TAILQ_EMPTY(&sprite->m_uses));

    assert(sprite->m_src->m_product == sprite);
    sprite->m_src->m_product = NULL;

    mem_free(mgr->m_alloc, sprite);
}
