#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/model/ui_data_module.h"
#include "ui/model/ui_data_sprite.h"
#include "ui/model/ui_data_action.h"
#include "ui/model/ui_data_layout.h"
#include "ui/model/ui_data_particle.h"
#include "ui_ed_obj_meta_i.h"
#include "ui_ed_mgr_i.h"

extern char g_metalib_ui_model_ed[];

extern ui_ed_obj_t ui_ed_frame_img_load_rel_img(ui_ed_obj_t obj);
extern ui_ed_obj_t ui_ed_obj_create_img_block(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_actor(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_actor_layer(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_actor_frame(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_particle_emitter(ui_ed_obj_t parent);
extern ui_ed_obj_t ui_ed_obj_create_particle_mod(ui_ed_obj_t parent);

#define ui_ed_obj_meta_init(__t, __meta) \
    obj_meta = &ed_mgr->m_metas[__t - UI_ED_OBJ_TYPE_MIN]; \
    obj_meta->m_type = __t;                                \
    obj_meta->m_data_meta = __meta;                        \
    assert(obj_meta->m_data_meta)

#define  ui_ed_obj_meta_set_id(__id, __id_set_fun)          \
    obj_meta->m_id_entry = dr_meta_find_entry_by_name(obj_meta->m_data_meta, __id); \
    assert(obj_meta->m_id_entry);                                        \
    obj_meta->m_set_id = (ui_ed_obj_set_id_fun_t )__id_set_fun

#define  ui_ed_obj_meta_set_delete(__delete_fun)          \
    obj_meta->m_delete = (ui_ed_obj_delete_fun_t)__delete_fun

#define  ui_ed_obj_meta_set_child(__ct, __fun) \
    obj_meta->m_child_create[__ct - UI_ED_OBJ_TYPE_MIN] = __fun

#define  ui_ed_obj_meta_set_rel(__rel_type, __load_fun) \
    obj_meta->m_rels[__rel_type - UI_ED_REL_TYPE_MIN].m_type = __rel_type; \
    obj_meta->m_rels[__rel_type - UI_ED_REL_TYPE_MIN].m_load = __load_fun

void ui_ed_obj_mgr_init_metas(ui_ed_mgr_t ed_mgr) {
    ui_ed_obj_meta_t obj_meta;

    bzero(ed_mgr->m_metas, sizeof(ed_mgr->m_metas));

    /*src*/
    ui_ed_obj_meta_init(ui_ed_obj_src, dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_ui_model_ed, "ui_ed_src"));
    ui_ed_obj_meta_set_child(ui_ed_obj_actor, ui_ed_obj_create_actor);
    ui_ed_obj_meta_set_child(ui_ed_obj_particle_emitter, ui_ed_obj_create_particle_emitter);

    /*module*/
    ui_ed_obj_meta_init(ui_ed_obj_module, ui_data_module_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_child(ui_ed_obj_img_block, ui_ed_obj_create_img_block);

    /*img_block*/
    ui_ed_obj_meta_init(ui_ed_obj_img_block, ui_data_img_block_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_id("id", ui_data_img_block_set_id);
    ui_ed_obj_meta_set_delete(ui_data_img_block_free);

    /*frame*/
    ui_ed_obj_meta_init(ui_ed_obj_frame, ui_data_frame_meta(ed_mgr->m_data_mgr));

    /*frame_img*/
    ui_ed_obj_meta_init(ui_ed_obj_frame_img, ui_data_frame_img_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_rel(ui_ed_rel_type_use_img, ui_ed_frame_img_load_rel_img);

    /*actor*/
    ui_ed_obj_meta_init(ui_ed_obj_actor, ui_data_actor_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_delete(ui_data_actor_free);
    ui_ed_obj_meta_set_child(ui_ed_obj_actor_layer, ui_ed_obj_create_actor_layer);

    /*actor_layer*/
    ui_ed_obj_meta_init(ui_ed_obj_actor_layer, ui_data_actor_layer_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_delete(ui_data_actor_layer_free);
    ui_ed_obj_meta_set_child(ui_ed_obj_actor_frame, ui_ed_obj_create_actor_frame);

    /*actor_frame*/
    ui_ed_obj_meta_init(ui_ed_obj_actor_frame, ui_data_actor_frame_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_delete(ui_data_actor_frame_free);

    /*particle_emitter*/
    ui_ed_obj_meta_init(ui_ed_obj_particle_emitter, ui_data_particle_emitter_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_delete(ui_data_particle_emitter_free);
    ui_ed_obj_meta_set_child(ui_ed_obj_particle_mod, ui_ed_obj_create_particle_mod);

    /*particle_mod*/
    ui_ed_obj_meta_init(ui_ed_obj_particle_mod, ui_data_particle_mod_meta(ed_mgr->m_data_mgr));
    ui_ed_obj_meta_set_delete(ui_data_particle_mod_free);
}

void ui_ed_obj_mgr_fini_metas(ui_ed_mgr_t ed_mgr) {
    uint8_t i;

    for(i = UI_ED_OBJ_TYPE_MIN; i < UI_ED_OBJ_TYPE_MAX; ++i) {
        ui_ed_obj_meta_t obj_meta = &ed_mgr->m_metas[i - UI_ED_OBJ_TYPE_MIN];
        bzero(obj_meta, sizeof(*obj_meta));
    }
}
