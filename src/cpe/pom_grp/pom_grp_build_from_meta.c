#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta_build.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"

pom_grp_meta_t
pom_grp_meta_build_from_meta(mem_allocrator_t alloc, uint32_t omm_page_size, LPDRMETA dr_meta, error_monitor_t em) {
    pom_grp_meta_t meta;
    int rv;
    size_t i;

    meta = pom_grp_meta_create(alloc, dr_meta_name(dr_meta), omm_page_size);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_from_cfg: create pom_grp_meta fail!");
        return NULL;
    }

    rv = 0;

    for(i = 0; i < dr_meta_entry_num(dr_meta); ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(dr_meta, i);
    /*     const char * entry_type;  */

    /*     if (cfg_child_count(entry_cfg) != 1) { */
    /*         CPE_ERROR(em, "pom_grp_meta_build_from_cfg: entry config child-count error!"); */
    /*         ++rv; */
    /*         continue; */
    /*     } */

    /*     entry_cfg = cfg_child_only(entry_cfg); */

    /*     entry_type = cfg_get_string(entry_cfg, "entry-type", NULL); */
    /*     if (strcmp(entry_type, "normal") == 0) { */
    /*         if (pom_grp_meta_build_from_cfg_entry_normal(meta, entry_cfg, metalib, em) != 0) { */
    /*             ++rv; */
    /*         } */
    /*     } */
    /*     else if (strcmp(entry_type, "list") == 0){ */
    /*         if (pom_grp_meta_build_from_cfg_entry_list(meta, entry_cfg, metalib, em) != 0) { */
    /*             ++rv; */
    /*         } */
    /*     } */
    /*     else if (strcmp(entry_type, "ba") == 0) { */
    /*         if (pom_grp_meta_build_from_cfg_entry_ba(meta, entry_cfg, metalib, em) != 0) { */
    /*             ++rv; */
    /*         } */
    /*     } */
    /*     else if (strcmp(entry_type, "binary") == 0) { */
    /*         if (pom_grp_meta_build_from_cfg_entry_binary(meta, entry_cfg, metalib, em) != 0) { */
    /*             ++rv; */
    /*         } */
    /*     } */
    /*     else { */
    /*         CPE_ERROR( */
    /*             em, "pom_grp_meta_build_from_cfg: entry %s: entry-type %s unknown!", */
    /*             cfg_name(entry_cfg), entry_type); */
    /*         ++rv; */
    /*         continue; */
    /*     } */
    /* } */

    /* if ((main_entry = cfg_get_string(cfg, "main-entry", NULL))) { */
    /*     if (pom_grp_meta_set_main_entry(meta, main_entry) != 0) { */
    /*         CPE_ERROR( */
    /*             em, "pom_grp_meta_build_from_cfg: set main_entry %s fail!", main_entry); */
    /*         rv = -1; */
    /*     } */
    /* } */
    }

    if (rv == 0) {
        return meta;
    }
    else {
        pom_grp_meta_free(meta);
        return NULL;
    }
}

