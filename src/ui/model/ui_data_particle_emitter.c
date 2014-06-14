#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/model/ui_data_src.h"
#include "ui_data_particle_i.h"
#include "ui_data_src_i.h"

ui_data_particle_emitter_t ui_data_particle_emitter_create(ui_data_particle_t particle) {
    ui_data_mgr_t mgr = particle->m_mgr;
    ui_data_particle_emitter_t emitter;

    emitter = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_particle_emitter));
    if (emitter == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in particle %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, particle->m_src));
        return NULL;
    }

    emitter->m_particle = particle;
    bzero(&emitter->m_data, sizeof(emitter->m_data));

    TAILQ_INIT(&emitter->m_mods);

    TAILQ_INSERT_TAIL(&particle->m_emitters, emitter, m_next_for_particle);

    return emitter;
}

void ui_data_particle_emitter_free(ui_data_particle_emitter_t emitter) {
    ui_data_particle_t particle = emitter->m_particle;
    ui_data_mgr_t mgr = particle->m_mgr;

    while(!TAILQ_EMPTY(&emitter->m_mods)) {
        ui_data_particle_mod_free(TAILQ_FIRST(&emitter->m_mods));
    }

    TAILQ_REMOVE(&particle->m_emitters, emitter, m_next_for_particle);

    mem_free(mgr->m_alloc, emitter);
}

static ui_data_particle_emitter_t ui_data_particle_emitter_in_particle_next(struct ui_data_particle_emitter_it * it) {
    ui_data_particle_emitter_t * data = (ui_data_particle_emitter_t *)(it->m_data);
    ui_data_particle_emitter_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_particle);

    return r;
}

void ui_data_particle_emitters(ui_data_particle_emitter_it_t it, ui_data_particle_t particle) {
    *(ui_data_particle_emitter_t *)(it->m_data) = TAILQ_FIRST(&particle->m_emitters);
    it->next = ui_data_particle_emitter_in_particle_next;
}

UI_PARTICLE_EMITTER * ui_data_particle_emitter_data(ui_data_particle_emitter_t particle_emitter) {
    return &particle_emitter->m_data;
}

LPDRMETA ui_data_particle_emitter_meta(ui_data_mgr_t data_mgr) {
    return data_mgr->m_meta_particle_emitter;
}
