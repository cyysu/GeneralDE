#ifndef UI_DATA_PARTICLE_INTERNAL_H
#define UI_DATA_PARTICLE_INTERNAL_H
#include "ui/model/ui_data_particle.h"
#include "ui_model_internal_types.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_data_particle_emitter_list, ui_data_particle_emitter) ui_data_particle_emitter_list_t;
typedef TAILQ_HEAD(ui_data_particle_mod_list, ui_data_particle_mod) ui_data_particle_mod_list_t;

struct ui_data_particle {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    ui_data_particle_emitter_list_t m_emitters;
};

struct ui_data_particle_emitter {
    ui_data_particle_t m_particle;
    TAILQ_ENTRY(ui_data_particle_emitter) m_next_for_particle;
    ui_data_particle_mod_list_t m_mods;
    UI_PARTICLE_EMITTER m_data;
};

struct ui_data_particle_mod {
    ui_data_particle_emitter_t m_emitter;
    TAILQ_ENTRY(ui_data_particle_mod) m_next_for_emitter;
    UI_PARTICLE_MOD m_data;
};

#ifdef __cplusplus
}
#endif

#endif
