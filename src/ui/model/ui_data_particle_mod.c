#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/model/ui_data_src.h"
#include "ui_data_particle_i.h"
#include "ui_data_src_i.h"

ui_data_particle_mod_t ui_data_particle_mod_create(ui_data_particle_emitter_t emitter) {
    ui_data_mgr_t mgr = emitter->m_particle->m_mgr;
    ui_data_particle_mod_t mod;

    mod = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_particle_mod));
    if (mod == NULL) {
        CPE_ERROR(
            mgr->m_em, "create particle mod in particle %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, emitter->m_particle->m_src));
        return NULL;
    }

    mod->m_emitter = emitter;
    bzero(&mod->m_data, sizeof(mod->m_data));

    TAILQ_INSERT_TAIL(&emitter->m_mods, mod, m_next_for_emitter);

    return mod;
}

void ui_data_particle_mod_free(ui_data_particle_mod_t mod) {
    ui_data_particle_emitter_t emitter = mod->m_emitter;
    ui_data_mgr_t mgr = emitter->m_particle->m_mgr;

    TAILQ_REMOVE(&emitter->m_mods, mod, m_next_for_emitter);

    mem_free(mgr->m_alloc, mod);
}

static ui_data_particle_mod_t ui_data_particle_mod_in_emitter_next(struct ui_data_particle_mod_it * it) {
    ui_data_particle_mod_t * data = (ui_data_particle_mod_t *)(it->m_data);
    ui_data_particle_mod_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_emitter);

    return r;
}

void ui_data_particle_emitter_mods(ui_data_particle_mod_it_t it, ui_data_particle_emitter_t emitter) {
    *(ui_data_particle_mod_t *)(it->m_data) = TAILQ_FIRST(&emitter->m_mods);
    it->next = ui_data_particle_mod_in_emitter_next;
}

UI_PARTICLE_MOD * ui_data_particle_mod_data(ui_data_particle_mod_t particle_mod) {
    return &particle_mod->m_data;
}

LPDRMETA ui_data_particle_mod_meta(ui_data_mgr_t data_mgr) {
    return data_mgr->m_meta_particle_mod;
}
