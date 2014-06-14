#ifndef UI_MODEL_DATA_PARTICLE_H
#define UI_MODEL_DATA_PARTICLE_H
#include "protocol/ui/model/ui_particle.h"
#include "protocol/ui/model/ui_particle_mod.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_particle_mod_it {
    ui_data_particle_mod_t (*next)(struct ui_data_particle_mod_it * it);
    char m_data[64];
};

struct ui_data_particle_emitter_it {
    ui_data_particle_emitter_t (*next)(struct ui_data_particle_emitter_it * it);
    char m_data[64];
};

/*particle*/
ui_data_particle_t ui_data_particle_create(ui_data_mgr_t mgr, ui_data_src_t src);
void ui_data_particle_free(ui_data_particle_t particle);
void ui_data_particle_emitters(ui_data_particle_emitter_it_t it, ui_data_particle_t particle);

/*particle_emitter*/
ui_data_particle_emitter_t ui_data_particle_emitter_create(ui_data_particle_t particle);
void ui_data_particle_emitter_free(ui_data_particle_emitter_t emitter);

UI_PARTICLE_EMITTER * ui_data_particle_emitter_data(ui_data_particle_emitter_t emitter);
LPDRMETA ui_data_particle_emitter_meta(ui_data_mgr_t data_mgr);
void ui_data_particle_emitter_mods(ui_data_particle_mod_it_t it, ui_data_particle_emitter_t emitter);

#define ui_data_particle_emitter_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*particle_mod*/
ui_data_particle_mod_t ui_data_particle_mod_create(ui_data_particle_emitter_t emitter);
void ui_data_particle_mod_free(ui_data_particle_mod_t particle_mod);
 
UI_PARTICLE_MOD * ui_data_particle_mod_data(ui_data_particle_mod_t particle_mod);
LPDRMETA ui_data_particle_mod_meta(ui_data_mgr_t data_mgr);

#define ui_data_particle_mod_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
