#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"
#include "pom_grp_data.h"

int pom_grp_obj_mgr_buf_init(
    LPDRMETALIB metalib,
    pom_grp_meta_t meta,
    void * data, size_t data_capacity,
    error_monitor_t em)
{
    struct pom_grp_obj_control_data * control;
    pom_mgr_t omm;

    size_t total_head_size;

    total_head_size
        = sizeof(struct pom_grp_obj_control_data)
        + CPE_PAL_ALIGN(pom_grp_entry_meta_calc_bin_size(meta))
        + CPE_PAL_ALIGN(dr_lib_size(metalib));

    if (total_head_size >= data_capacity) {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create_by_init: data buf too small! require "FMT_SIZE_T", but only "FMT_SIZE_T""
            ": control size "FMT_SIZE_T", om-meta size "FMT_SIZE_T", metalib size "FMT_SIZE_T"",
            total_head_size, data_capacity,
            sizeof(struct pom_grp_obj_control_data),
            CPE_PAL_ALIGN(pom_grp_entry_meta_calc_bin_size(meta)),
            CPE_PAL_ALIGN(dr_lib_size(metalib)));
        return -1;
    }

    control = (struct pom_grp_obj_control_data *)data;

    control->m_magic = OM_GRP_OBJ_CONTROL_MAGIC;
    control->m_head_version = 1;

    control->m_objmeta_start = sizeof(struct pom_grp_obj_control_data);
    control->m_objmeta_size = pom_grp_entry_meta_calc_bin_size(meta);
    pom_grp_entry_meta_write_to_bin(((char *)data) + control->m_objmeta_start, control->m_objmeta_size, meta);

    control->m_metalib_start = control->m_objmeta_start + CPE_PAL_ALIGN(control->m_objmeta_size);
    control->m_metalib_size = dr_lib_size(metalib);
    memcpy(((char *)data) + control->m_metalib_start, metalib, control->m_metalib_size);

    control->m_data_start = control->m_metalib_start + CPE_PAL_ALIGN(control->m_metalib_size);
    control->m_data_size = data_capacity - control->m_data_start;

    omm = pom_mgr_create(NULL, meta->m_omm_page_size, control->m_data_size);
    if (omm == NULL) {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create_by_init: create omm for init data buf fail, page-size=%d, buf-size="FMT_SIZE_T"!",
            meta->m_omm_page_size, control->m_data_size);
        return -1;
    }

    if (pom_mgr_add_new_buffer(
            omm,
            (pom_buffer_id_t)(((char*)data) + control->m_data_start),
            em) != 0)
    {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create_by_init: create omm for init data buf fail, page-size=%d, buf-size="FMT_SIZE_T"!",
            meta->m_omm_page_size, control->m_data_size);
        pom_mgr_free(omm);
        return -1;
    }

    pom_mgr_free(omm);
    return 0;
}

