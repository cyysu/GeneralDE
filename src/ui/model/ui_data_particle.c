#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui_data_particle_i.h"

ui_data_particle_t ui_data_particle_create(ui_data_mgr_t mgr, ui_data_src_t src) {
    ui_data_particle_t particle;

    if (src->m_type != ui_data_src_type_particle) {
        CPE_ERROR(
            mgr->m_em, "create particle at %s: src not particle!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    if (src->m_product) {
        CPE_ERROR(
            mgr->m_em, "create particle at %s: product already loaded!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    particle = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_particle));
    if (particle == NULL) {
        CPE_ERROR(
            mgr->m_em, "create particle at %s: alloc fail!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    src->m_product = particle;
    particle->m_mgr = mgr;
    particle->m_src = src;
    TAILQ_INIT(&particle->m_emitters);

    return particle;
}

void ui_data_particle_free(ui_data_particle_t particle) {
    ui_data_mgr_t mgr = particle->m_mgr;

    assert(particle->m_src->m_product == particle);
    particle->m_src->m_product = NULL;

    while(!TAILQ_EMPTY(&particle->m_emitters)) {
        ui_data_particle_emitter_free(TAILQ_FIRST(&particle->m_emitters));
    }

    mem_free(mgr->m_alloc, particle);
}
