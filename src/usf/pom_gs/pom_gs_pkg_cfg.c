#include <assert.h>
#include "cpe/cfg/cfg.h"
#include "cpe/dr/dr_cfg.h" 
#include "cpe/dr/dr_metalib_manage.h" 
#include "cpe/pom_grp/pom_grp_store.h" 
#include "usf/pom_gs/pom_gs_pkg.h"
#include "usf/pom_gs/pom_gs_pkg_cfg.h"
#include "pom_gs_internal_ops.h"

int pom_gs_pkg_cfg_dump(cfg_t cfg, pom_gs_pkg_t pkg, error_monitor_t em) {
    uint32_t i;
    struct pom_gs_pkg_data_entry * data_entry;
    cfg_t child_cfg;
    int rv;

    rv = 0;

    for(i = 0; i < pkg->m_entry_count; ++i) {
        data_entry = &pkg->m_entries[i];

        if (data_entry->m_capacity <= 0) continue;

        child_cfg = cfg_struct_add_struct(cfg, pom_grp_store_table_name(data_entry->m_table), cfg_replace);
        if (child_cfg == NULL) {
            CPE_ERROR(
                em, "pom_gs_pkg_cfg_dump: %s: create sub cfg fail!",
                pom_grp_store_table_name(data_entry->m_table));
            rv = -1;
            continue;
        }

        if (dr_cfg_write(
                child_cfg, 
                pom_gs_pkg_entry_buf(pkg, data_entry),
                pom_grp_store_table_meta(data_entry->m_table), em) != 0)
        {
            CPE_ERROR(
                em, "pom_grp_obj_cfg_dump: %s: dump date fail!", 
                pom_grp_store_table_name(data_entry->m_table));
            rv = -1;
            continue;
        }
    }

    return rv;
}

int pom_gs_pkg_cfg_load(pom_gs_pkg_t pkg, cfg_t cfg, error_monitor_t em) {
    int i;
    struct pom_gs_pkg_data_entry * data_entry;
    cfg_t child_cfg;
    LPDRMETA data_meta;
    size_t capacity;
    void * buf;
    int rv;

    rv = 0;

    for(i = 0; i < pkg->m_entry_count; ++i) {
        data_entry = &pkg->m_entries[i];

        child_cfg = cfg_struct_find_cfg(cfg, pom_grp_store_table_name(data_entry->m_table));
        if (child_cfg == NULL) {
            pom_gs_pkg_buf_resize(pkg, data_entry, 0);
            continue;
        }

        data_meta = pom_grp_store_table_meta(data_entry->m_table);
        capacity = dr_meta_size(data_meta);
        if (pom_gs_pkg_buf_resize(pkg, data_entry, capacity) != 0) {
            CPE_ERROR(
                em, "pom_gs_pkg_load: %s: resize fail, capacitiy=%d!",
                pom_grp_store_table_name(data_entry->m_table), (int)capacity);
            rv = -1;
            continue;
        }

        buf = pom_gs_pkg_entry_buf(pkg, data_entry);
        if (dr_cfg_read(buf, capacity, cfg, data_meta, 0, em) != 0) {
            CPE_ERROR(
                em, "pom_grp_pkg_load: %s: read date fail!", 
                pom_grp_store_table_name(data_entry->m_table));
            rv = -1;
            continue;
        }
    }

    return rv;
}

int pom_gs_pkg_dump_to_stream(write_stream_t stream, pom_gs_pkg_t pkg, error_monitor_t em) {
    cfg_t root_cfg;
    int rv;

    root_cfg = cfg_create(NULL);
    if (root_cfg == NULL) {
        CPE_ERROR(em, "pom_gs_pkg_dump_to_stream: create root cfg fail!");
        return -1;
    }

    rv = 0;

    rv = pom_gs_pkg_cfg_dump(root_cfg, pkg, em);
    if (cfg_write(stream, root_cfg, em) != 0){
        CPE_ERROR(em, "pom_gs_pkg_dump_to_stream: write cfg fail!");
        rv = -1;
        goto COMPLETE; 
    }

COMPLETE:
    cfg_free(root_cfg);
    return rv;
}
