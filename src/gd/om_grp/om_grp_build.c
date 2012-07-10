#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/om_grp/om_grp_meta_build.h"
#include "gd/om_grp/om_grp_meta.h"
#include "om_grp_internal_ops.h"

static int om_grp_meta_build_from_cfg_entry_normal(
    om_grp_meta_t meta, cfg_t entry_cfg, LPDRMETALIB metalib, error_monitor_t em)
{
    const char * data_type;
    LPDRMETA data_meta;

    data_type  = cfg_get_string(entry_cfg, "data-type", NULL);
    if (data_type == NULL) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: entry-normal not configure data-type!",
            cfg_name(entry_cfg));
        return -1;
    }

    data_meta = dr_lib_find_meta_by_name(metalib, data_type);
    if (data_meta == NULL) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: entry-normal not configure data-type!",
            cfg_name(entry_cfg));
        return -1;
    }

    if (om_grp_entry_meta_normal_create(meta, cfg_name(entry_cfg), data_meta) == NULL) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: create normal entry fail!",
            cfg_name(entry_cfg));
        return -1;
    }

    return 0;
}

static int om_grp_meta_build_from_cfg_entry_list(
    om_grp_meta_t meta, cfg_t entry_cfg, LPDRMETALIB metalib, error_monitor_t em)
{
    const char * data_type;
    LPDRMETA data_meta;
    const char * data_capacity;
    int list_count;

    data_type  = cfg_get_string(entry_cfg, "data-type", NULL);
    if (data_type == NULL) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: entry-list not configure data-type!",
            cfg_name(entry_cfg));
        return -1;
    }

    data_meta = dr_lib_find_meta_by_name(metalib, data_type);
    if (data_meta == NULL) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: entry-list not configure data-type!",
            cfg_name(entry_cfg));
        return -1;
    }

    list_count = cfg_get_int32(entry_cfg, "capacity", -1);
    if (list_count == -1) {
        data_capacity = cfg_get_string(entry_cfg, "capacity", NULL);
        if (data_capacity == NULL) {
            CPE_ERROR(
                em, "om_grp_meta_build_from_cfg: entry %s: entry-list not configure capacity!",
                cfg_name(entry_cfg));
            return -1;
        }

        if (dr_lib_find_macro_value(&list_count, metalib, data_capacity) != 0) {
            CPE_ERROR(
                em, "om_grp_meta_build_from_cfg: entry %s: entry-list capacity %s not exist in metalib!",
                cfg_name(entry_cfg), data_capacity);
            return -1;
        }
    }

    if (list_count <= 0) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: entry-list capacity %d error!",
            cfg_name(entry_cfg), list_count);
        return -1;
    }

    if (om_grp_entry_meta_list_create(meta, cfg_name(entry_cfg), data_meta, list_count) == NULL) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: create list entry fail!",
            cfg_name(entry_cfg));
        return -1;
    }

    return 0;
}

static int om_grp_meta_build_from_cfg_entry_ba(
    om_grp_meta_t meta, cfg_t entry_cfg, LPDRMETALIB metalib, error_monitor_t em)
{
    const char * data_capacity;
    int ba_count;

    ba_count = cfg_get_int32(entry_cfg, "capacity", -1);
    if (ba_count == -1) {
        data_capacity = cfg_get_string(entry_cfg, "capacity", NULL);
        if (data_capacity == NULL) {
            CPE_ERROR(
                em, "om_grp_meta_build_from_cfg: entry %s: entry-ba not configure capacity!",
                cfg_name(entry_cfg));
            return -1;
        }

        if (dr_lib_find_macro_value(&ba_count, metalib, data_capacity) != 0) {
            CPE_ERROR(
                em, "om_grp_meta_build_from_cfg: entry %s: entry-ba capacity %s not exist in metalib!",
                cfg_name(entry_cfg), data_capacity);
            return -1;
        }
    }

    if (ba_count <= 0) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: entry-ba capacity %d error!",
            cfg_name(entry_cfg), ba_count);
        return -1;
    }

    if (om_grp_entry_meta_ba_create(meta, cfg_name(entry_cfg), ba_count) == NULL) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: create ba entry fail!",
            cfg_name(entry_cfg));
        return -1;
    }

    return 0;
}
 
static int om_grp_meta_build_from_cfg_entry_binary(
    om_grp_meta_t meta, cfg_t entry_cfg, LPDRMETALIB metalib, error_monitor_t em)
{
    const char * data_capacity;
    int binary_count;

    binary_count = cfg_get_int32(entry_cfg, "capacity", -1);
    if (binary_count == -1) {
        data_capacity = cfg_get_string(entry_cfg, "capacity", NULL);
        if (data_capacity == NULL) {
            CPE_ERROR(
                em, "om_grp_meta_build_from_cfg: entry %s: entry-binary not configure capacity!",
                cfg_name(entry_cfg));
            return -1;
        }

        if (dr_lib_find_macro_value(&binary_count, metalib, data_capacity) != 0) {
            CPE_ERROR(
                em, "om_grp_meta_build_from_cfg: entry %s: entry-binary capacity %s not exist in metalib!",
                cfg_name(entry_cfg), data_capacity);
            return -1;
        }
    }

    if (binary_count <= 0) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: entry-binary capacity %d error!",
            cfg_name(entry_cfg), binary_count);
        return -1;
    }

    if (om_grp_entry_meta_binary_create(meta, cfg_name(entry_cfg), binary_count) == NULL) {
        CPE_ERROR(
            em, "om_grp_meta_build_from_cfg: entry %s: create binary entry fail!",
            cfg_name(entry_cfg));
        return -1;
    }

    return 0;
}
 
om_grp_meta_t
om_grp_meta_build_from_cfg(mem_allocrator_t alloc, cfg_t cfg, LPDRMETALIB metalib, error_monitor_t em) {
    om_grp_meta_t meta;
    struct cfg_it entry_it;
    cfg_t entry_cfg;
    int rv;

    meta = om_grp_meta_create(alloc, cfg_name(cfg));
    if (meta == NULL) {
        CPE_ERROR(em, "om_grp_meta_build_from_cfg: create om_grp_meta fail!");
        return NULL;
    }

    rv = 0;

    cfg_it_init(&entry_it, cfg);
    while((entry_cfg = cfg_it_next(&entry_it))) {
        const char * entry_type; 

        if (cfg_child_count(entry_cfg) != 1) {
            CPE_ERROR(em, "om_grp_meta_build_from_cfg: entry config child-count error!");
            ++rv;
            continue;
        }

        entry_cfg = cfg_child_only(entry_cfg);

        entry_type = cfg_get_string(entry_cfg, "entry-type", NULL);
        if (strcmp(entry_type, "normal") == 0) {
            if (om_grp_meta_build_from_cfg_entry_normal(meta, entry_cfg, metalib, em) != 0) {
                ++rv;
            }
        }
        else if (strcmp(entry_type, "list") == 0){
            if (om_grp_meta_build_from_cfg_entry_list(meta, entry_cfg, metalib, em) != 0) {
                ++rv;
            }
        }
        else if (strcmp(entry_type, "ba") == 0) {
            if (om_grp_meta_build_from_cfg_entry_ba(meta, entry_cfg, metalib, em) != 0) {
                ++rv;
            }
        }
        else if (strcmp(entry_type, "binary") == 0) {
            if (om_grp_meta_build_from_cfg_entry_binary(meta, entry_cfg, metalib, em) != 0) {
                ++rv;
            }
        }
        else {
            CPE_ERROR(
                em, "om_grp_meta_build_from_cfg: entry %s: entry-type %s unknown!",
                cfg_name(entry_cfg), entry_type);
            ++rv;
            continue;
        }
    }

    if (rv == 0) {
        return meta;
    }
    else {
        om_grp_meta_free(meta);
        return NULL;
    }
}

