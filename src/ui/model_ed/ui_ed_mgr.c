#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/model/ui_data_mgr.h"
#include "ui/model/ui_data_src.h"
#include "ui_ed_mgr_i.h"
#include "ui_ed_src_i.h"
#include "ui_ed_obj_i.h"
#include "ui_ed_obj_meta_i.h"

ui_ed_mgr_t ui_ed_mgr_create(mem_allocrator_t alloc, ui_data_mgr_t data_mgr, error_monitor_t em) {
    ui_ed_mgr_t ed_mgr;

    ed_mgr = mem_alloc(alloc, sizeof(struct ui_ed_mgr));
    if (ed_mgr == NULL) {
        CPE_ERROR(em, "create ui_ed fail!");
        return NULL;
    }

    ed_mgr->m_alloc = alloc;
    ed_mgr->m_em = em;
    ed_mgr->m_data_mgr = data_mgr;

    mem_buffer_init(&ed_mgr->m_dump_buffer, alloc);

    if (cpe_hash_table_init(
            &ed_mgr->m_ed_srcs,
            alloc,
            (cpe_hash_fun_t) ui_ed_src_hash,
            (cpe_hash_cmp_t) ui_ed_src_eq,
            CPE_HASH_OBJ2ENTRY(ui_ed_src, m_hh_for_mgr),
            -1) != 0)
    {
        mem_buffer_clear(&ed_mgr->m_dump_buffer);
        mem_free(alloc, ed_mgr);
        return NULL;
    }

    if (cpe_hash_table_init(
            &ed_mgr->m_ed_objs,
            alloc,
            (cpe_hash_fun_t) ui_ed_obj_hash,
            (cpe_hash_cmp_t) ui_ed_obj_eq,
            CPE_HASH_OBJ2ENTRY(ui_ed_obj, m_hh_for_mgr),
            -1) != 0)
    {
        cpe_hash_table_fini(&ed_mgr->m_ed_srcs);
        mem_buffer_clear(&ed_mgr->m_dump_buffer);
        mem_free(alloc, ed_mgr);
        return NULL;
    }

    ui_ed_obj_mgr_init_metas(ed_mgr);

    return ed_mgr;
}

void ui_ed_mgr_free(ui_ed_mgr_t ed_mgr) {
    ui_ed_src_free_all(ed_mgr);
    ui_ed_obj_mgr_fini_metas(ed_mgr);

    assert(cpe_hash_table_count(&ed_mgr->m_ed_srcs) == 0);
    cpe_hash_table_fini(&ed_mgr->m_ed_srcs);

    assert(cpe_hash_table_count(&ed_mgr->m_ed_objs) == 0);
    cpe_hash_table_fini(&ed_mgr->m_ed_objs);

    mem_buffer_clear(&ed_mgr->m_dump_buffer);
    mem_free(ed_mgr->m_alloc, ed_mgr);
}

int ui_ed_mgr_save(ui_ed_mgr_t ed_mgr, const char * root, error_monitor_t em) {
    struct cpe_hash_it src_it;
    ui_ed_src_t src;
    int r = 0;

    cpe_hash_it_init(&src_it, &ed_mgr->m_ed_srcs);

    src = cpe_hash_it_next(&src_it);
    while (src) {
        ui_ed_src_t next = cpe_hash_it_next(&src_it);

        switch(src->m_state) {
        case ui_ed_src_state_normal:
            break;
        case ui_ed_src_state_new:
        case ui_ed_src_state_changed:
            if (ui_ed_src_save(src, root, em) != 0) r = -1;
            break;
        case ui_ed_src_state_removed: {
            ui_data_src_t data_src = src->m_data_src;
            ui_ed_src_free_i(src);
            ui_data_src_remove(data_src, root, em);
            break;
        }
        }

        src = next;
    }

    return r;
}
