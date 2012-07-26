#include <assert.h>
#include "cpe/cfg/cfg_manage.h" 
#include "cpe/cfg/cfg_read.h" 
#include "cpe/dr/dr_cfg.h" 
#include "cpe/pom_grp/pom_grp_cfg.h"
#include "cpe/pom_grp/pom_grp_obj.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"

static int pom_grp_obj_cfg_dump_normal(
    cfg_t cfg,
    pom_grp_meta_t meta,
    pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr,
    pom_grp_obj_t obj,
    error_monitor_t em)
{
    void * data;

    data = pom_grp_obj_normal_ex(mgr, obj, entry_meta);
    if (data == NULL) return 0;

    cfg = cfg_struct_add_struct(cfg, entry_meta->m_name, cfg_replace);
    if (cfg == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: create sub cfg fail!", entry_meta->m_name);
        return -1;
    }

    if (dr_cfg_write(cfg, data, pom_grp_entry_meta_normal_meta(entry_meta), em) != 0) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: dump date fail!", entry_meta->m_name);
        return -1;
    }

    return 0;
}

static int pom_grp_obj_cfg_dump_list(
    cfg_t cfg,
    pom_grp_meta_t meta,
    pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr,
    pom_grp_obj_t obj,
    error_monitor_t em)
{
    uint16_t i, count;
    LPDRMETA data_meta;
    cfg_t data_cfg;
    void * data;
    int rv;

    cfg = cfg_struct_add_seq(cfg, entry_meta->m_name, cfg_replace);
    if (cfg == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: create sub cfg fail!", entry_meta->m_name);
        return -1;
    }

    data_meta = pom_grp_entry_meta_list_meta(entry_meta);

    rv = 0;

    count = pom_grp_obj_list_count_ex(mgr, obj, entry_meta);
    for(i = 0; i < count; ++i) {
        data = pom_grp_obj_list_at_ex(mgr, obj, entry_meta, i);
        if (data == NULL) {
            CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: get list item %d fail!", entry_meta->m_name, i);
            rv = -1;
            continue;
        }

        data_cfg = cfg_seq_add_struct(cfg);
        if (dr_cfg_write(data_cfg, data, data_meta, em) != 0) {
            CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: dump list item %d fail!", entry_meta->m_name, i);
            rv = -1;
            continue;
        }
    }

    return rv;
}

int pom_grp_obj_cfg_dump(cfg_t cfg, pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em) {
    uint16_t i, count;
    int rv;
    pom_grp_meta_t meta;

    rv = 0;

    meta = pom_grp_obj_mgr_meta(mgr);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump: no meta!");
        return -1;
    }

    count =  pom_grp_meta_entry_count(meta);
    for(i = 0; i < count; ++i) {
        pom_grp_entry_meta_t entry_meta = pom_grp_meta_entry_at(meta, i);
        assert(entry_meta);

        switch(entry_meta->m_type) {
        case pom_grp_entry_type_normal:
            if (pom_grp_obj_cfg_dump_normal(cfg, meta, entry_meta, mgr, obj, em) != 0) rv = -1;
            break;
        case pom_grp_entry_type_list:
            if (pom_grp_obj_cfg_dump_list(cfg, meta, entry_meta, mgr, obj, em) != 0) rv = -1;
            break;
        case pom_grp_entry_type_ba:
            break;
        case pom_grp_entry_type_binary:
            break;
        }
    }

    return 0;
}

int pom_grp_obj_cfg_load(pom_grp_obj_t obj, pom_grp_obj_mgr_t mgr, cfg_t cfg, error_monitor_t em);

int pom_grp_obj_cfg_dump_all(cfg_t cfg, pom_grp_obj_mgr_t mgr, error_monitor_t em) {
    int rv;
    struct pom_grp_obj_it obj_it;
    pom_grp_obj_t obj;

    if (cfg_type(cfg) != CPE_CFG_TYPE_SEQUENCE) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_all: can`t dump to %d cfg!", cfg_type(cfg));
        return -1;
    }

    rv = 0;

    pom_grp_objs(mgr, &obj_it);

    while((obj = pom_grp_obj_it_next(&obj_it))) {
        cfg_t child_cfg = cfg_seq_add_struct(cfg);
        if (child_cfg == NULL) {
            CPE_ERROR(em, "pom_grp_obj_cfg_dump_all: create sub cfg fail!");
            return -1;
        }

        if (pom_grp_obj_cfg_dump(child_cfg, mgr, obj, em) != 0) rv = -1;
    }

    return rv;
}

int pom_grp_obj_cfg_load_all(pom_grp_obj_mgr_t mgr, cfg_t cfg, error_monitor_t em);

int pom_grp_obj_cfg_dump_to_stream(write_stream_t stream, pom_grp_obj_mgr_t mgr, error_monitor_t em) {
    cfg_t root_cfg;
    cfg_t dump_cfg;
    pom_grp_meta_t meta;
    int rv;

    meta = pom_grp_obj_mgr_meta(mgr);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: no meta!");
        return -1;
    }

    root_cfg = cfg_create(NULL);
    if (root_cfg == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: create root cfg fail!");
        return -1;
    }

    rv = 0;

    dump_cfg = cfg_struct_add_seq(root_cfg, pom_grp_meta_name(meta), cfg_replace);
    if (dump_cfg == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: create dump cfg fail!");
        rv = -1;
        goto COMPLETE; 
    }

    rv = pom_grp_obj_cfg_dump_all(dump_cfg, mgr, em);

    if (cfg_write(stream, dump_cfg, em) != 0){
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: write cfg fail!");
        rv = -1;
        goto COMPLETE; 
    }

COMPLETE:
    cfg_free(root_cfg);
    return rv;
}
