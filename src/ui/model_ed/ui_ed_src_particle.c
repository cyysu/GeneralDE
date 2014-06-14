#include <assert.h>
#include "ui/model/ui_data_src.h"
#include "ui/model/ui_data_particle.h"
#include "ui_ed_src_i.h"
#include "ui_ed_obj_i.h"

int ui_ed_src_load_particle(ui_ed_src_t ed_src) {
    ui_data_particle_t particle;
    struct ui_data_particle_emitter_it emitter_it;
    ui_data_particle_emitter_t emitter;

    particle = ui_data_src_product(ed_src->m_data_src);
    assert(particle);

    ui_data_particle_emitters(&emitter_it, particle);
    while((emitter = ui_data_particle_emitter_it_next(&emitter_it))) {
        ui_ed_obj_t emitter_obj;
        struct ui_data_particle_mod_it mod_it;
        ui_data_particle_mod_t mod;

        emitter_obj = ui_ed_obj_create_i(
            ed_src, ed_src->m_root_obj,
            ui_ed_obj_particle_emitter,
            emitter, ui_data_particle_emitter_data(emitter), sizeof(*ui_data_particle_emitter_data(emitter)));
        if (emitter_obj == NULL) return -1;

        ui_data_particle_emitter_mods(&mod_it, emitter);
        while((mod = ui_data_particle_mod_it_next(&mod_it))) {
            ui_ed_obj_t obj =
                ui_ed_obj_create_i(
                    ed_src, emitter_obj,
                    ui_ed_obj_particle_mod,
                    mod, ui_data_particle_mod_data(mod), sizeof(*ui_data_particle_mod_data(mod)));
            if (obj == NULL) return -1;
        }
    }
    
    return 0;
}

ui_ed_obj_t ui_ed_obj_create_particle_emitter(ui_ed_obj_t parent) {
    ui_data_particle_t particle;
    ui_data_particle_emitter_t particle_emitter;
    ui_ed_obj_t obj;

    particle = ui_ed_src_product(parent->m_src);
    assert(particle);

    particle_emitter = ui_data_particle_emitter_create(particle);
    if (particle_emitter == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        parent->m_src, parent,
        ui_ed_obj_particle_emitter,
        particle_emitter, ui_data_particle_emitter_data(particle_emitter), sizeof(*ui_data_particle_emitter_data(particle_emitter)));
    if (obj == NULL) {
        ui_data_particle_emitter_free(particle_emitter);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t ui_ed_obj_create_particle_mod(ui_ed_obj_t parent) {
    ui_data_particle_emitter_t emitter;
    ui_data_particle_mod_t mod;
    ui_ed_obj_t obj;

    emitter = ui_ed_obj_product(parent);
    assert(emitter);

    mod = ui_data_particle_mod_create(emitter);
    if (mod == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        parent->m_src, parent,
        ui_ed_obj_particle_mod,
        mod, ui_data_particle_mod_data(mod), sizeof(*ui_data_particle_mod_data(mod)));
    if (obj == NULL) {
        ui_data_particle_mod_free(mod);
        return NULL;
    }

    return obj;
}
