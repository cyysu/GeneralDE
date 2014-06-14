#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/model/ui_data_mgr.h"
#include "ui_model_internal_types.h"
#include "ui_data_src_i.h"
#include "ui_data_module_i.h"
#include "ui_data_layout_i.h"

extern char g_metalib_ui_model[];
extern void ui_data_mgr_product_init(ui_data_mgr_t mgr);

#define UI_DATA_MGR_LOAD_META(__arg, __name) \
    mgr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_ui_model, __name); \
    assert(mgr-> __arg)


ui_data_mgr_t ui_data_mgr_create(mem_allocrator_t alloc, const char * root, error_monitor_t em) {
    ui_data_mgr_t mgr;

    mgr = mem_alloc(alloc, sizeof(struct ui_data_mgr));
    if (mgr == NULL) {
        CPE_ERROR(em, "ui_data_mgr_create: allc fail!");
        return NULL;
    }

    mgr->m_alloc = alloc;
    mgr->m_em = em;
    ui_data_mgr_product_init(mgr);
    mem_buffer_init(&mgr->m_dump_buffer, alloc);

    UI_DATA_MGR_LOAD_META(m_meta_module, "ui_module");
    UI_DATA_MGR_LOAD_META(m_meta_img_block, "ui_img_block");
    UI_DATA_MGR_LOAD_META(m_meta_frame, "ui_frame");
    UI_DATA_MGR_LOAD_META(m_meta_frame_img, "ui_img_ref");
    UI_DATA_MGR_LOAD_META(m_meta_actor, "ui_actor");
    UI_DATA_MGR_LOAD_META(m_meta_actor_layer, "ui_actor_layer");
    UI_DATA_MGR_LOAD_META(m_meta_actor_frame, "ui_actor_frame");
    UI_DATA_MGR_LOAD_META(m_meta_particle_emitter, "ui_particle_emitter");
    UI_DATA_MGR_LOAD_META(m_meta_particle_mod, "ui_particle_mod");

    UI_DATA_MGR_LOAD_META(m_meta_controls[ui_data_control_window - UI_DATA_CONTROL_TYPE_MIN], "ui_window");
    UI_DATA_MGR_LOAD_META(m_meta_controls[ui_data_control_panel - UI_DATA_CONTROL_TYPE_MIN], "ui_panel");
    UI_DATA_MGR_LOAD_META(m_meta_controls[ui_data_control_picture - UI_DATA_CONTROL_TYPE_MIN], "ui_picture");
    UI_DATA_MGR_LOAD_META(m_meta_controls[ui_data_control_label - UI_DATA_CONTROL_TYPE_MIN], "ui_label");
    UI_DATA_MGR_LOAD_META(m_meta_controls[ui_data_control_button - UI_DATA_CONTROL_TYPE_MIN], "ui_button");
    UI_DATA_MGR_LOAD_META(m_meta_controls[ui_data_control_toggle - UI_DATA_CONTROL_TYPE_MIN], "ui_toggle");
    UI_DATA_MGR_LOAD_META(m_meta_controls[ui_data_control_progress - UI_DATA_CONTROL_TYPE_MIN], "ui_progress");
    UI_DATA_MGR_LOAD_META(m_meta_controls[ui_data_control_picture_cond - UI_DATA_CONTROL_TYPE_MIN], "ui_picture_cond");

    if (cpe_hash_table_init(
            &mgr->m_srcs,
            alloc,
            (cpe_hash_fun_t) ui_data_src_hash,
            (cpe_hash_cmp_t) ui_data_src_eq,
            CPE_HASH_OBJ2ENTRY(ui_data_src, m_hh_for_mgr),
            -1) != 0)
    {
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_free(alloc, mgr);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_srcs_by_id,
            alloc,
            (cpe_hash_fun_t) ui_data_src_id_hash,
            (cpe_hash_cmp_t) ui_data_src_id_eq,
            CPE_HASH_OBJ2ENTRY(ui_data_src, m_hh_for_mgr_id),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_srcs);
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_free(alloc, mgr);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_img_blocks,
            alloc,
            (cpe_hash_fun_t) ui_data_img_block_hash,
            (cpe_hash_cmp_t) ui_data_img_block_eq,
            CPE_HASH_OBJ2ENTRY(ui_data_img_block, m_hh_for_mgr),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_srcs);
        cpe_hash_table_fini(&mgr->m_srcs_by_id);
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_free(alloc, mgr);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_controls_by_id,
            alloc,
            (cpe_hash_fun_t) ui_data_control_hash,
            (cpe_hash_cmp_t) ui_data_control_eq,
            CPE_HASH_OBJ2ENTRY(ui_data_control, m_hh_for_mgr),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_img_blocks);
        cpe_hash_table_fini(&mgr->m_srcs);
        cpe_hash_table_fini(&mgr->m_srcs_by_id);
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_free(alloc, mgr);
        return NULL;
    }

    mgr->m_src_root = ui_data_src_create_i(mgr, NULL, ui_data_src_type_dir, root);
    if (mgr->m_src_root == NULL) {
        CPE_ERROR(em, "ui_data_mgr_create: alloc root fail!");
        cpe_hash_table_fini(&mgr->m_srcs);
        cpe_hash_table_fini(&mgr->m_srcs_by_id);
        cpe_hash_table_fini(&mgr->m_img_blocks);
        cpe_hash_table_fini(&mgr->m_controls_by_id);
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_free(alloc, mgr);
        return NULL;
    }

    return mgr;
}

void ui_data_mgr_free(ui_data_mgr_t mgr) {
    ui_data_src_free(mgr->m_src_root);
    mgr->m_src_root = NULL;

    assert(cpe_hash_table_count(&mgr->m_controls_by_id) == 0);
    cpe_hash_table_fini(&mgr->m_controls_by_id);

    assert(cpe_hash_table_count(&mgr->m_img_blocks) == 0);
    cpe_hash_table_fini(&mgr->m_img_blocks);

    assert(cpe_hash_table_count(&mgr->m_srcs) == 0);
    cpe_hash_table_fini(&mgr->m_srcs);

    assert(cpe_hash_table_count(&mgr->m_srcs_by_id) == 0);
    cpe_hash_table_fini(&mgr->m_srcs_by_id);

    mem_buffer_clear(&mgr->m_dump_buffer);

    mem_free(mgr->m_alloc, mgr);
}

ui_data_src_t ui_data_mgr_src_root(ui_data_mgr_t mgr) {
    return mgr->m_src_root;
}
