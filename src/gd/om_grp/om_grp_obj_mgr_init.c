#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "gd/om_grp/om_grp_obj_mgr.h"
#include "gd/om_grp/om_grp_meta.h"
#include "om_grp_internal_ops.h"
#include "om_grp_data.h"

int om_grp_obj_mgr_buf_init(
    LPDRMETALIB metalib,
    om_grp_meta_t meta,
    void * data, size_t data_capacity,
    error_monitor_t em)
{
    struct om_grp_obj_control_data * control;

    size_t total_head_size;

    total_head_size
        = sizeof(struct om_grp_obj_control_data)
        + CPE_PAL_ALIGN(om_grp_entry_meta_calc_bin_size(meta))
        + CPE_PAL_ALIGN(dr_lib_size(metalib));

    if (total_head_size >= data_capacity) {
        CPE_ERROR(
            em, "om_grp_obj_mgr_create_by_init: data buf too small! require "FMT_SIZE_T", but only "FMT_SIZE_T""
            "control size "FMT_SIZE_T", om-meta size "FMT_SIZE_T", metalib size "FMT_SIZE_T"",
            total_head_size, data_capacity,
            sizeof(struct om_grp_obj_control_data),
            CPE_PAL_ALIGN(om_grp_entry_meta_calc_bin_size(meta)),
            CPE_PAL_ALIGN(dr_lib_size(metalib)));
        return -1;
    }

    control = (struct om_grp_obj_control_data *)data;

    control->m_magic = OM_GRP_OBJ_CONTROL_MAGIC;
    control->m_head_version = 1;

    control->m_objmeta_start = sizeof(struct om_grp_obj_control_data);
    control->m_objmeta_size = om_grp_entry_meta_calc_bin_size(meta);
    om_grp_entry_meta_write_to_bin(((char *)data) + control->m_objmeta_start, control->m_objmeta_size, meta);

    control->m_metalib_start = control->m_objmeta_start + CPE_PAL_ALIGN(control->m_objmeta_size);
    control->m_metalib_size = dr_lib_size(metalib);
    memcpy(((char *)data) + control->m_metalib_start, metalib, control->m_metalib_size);

    control->m_data_start = control->m_metalib_start + CPE_PAL_ALIGN(control->m_metalib_size);
    control->m_data_size = data_capacity - control->m_data_start;

    return 0;
}

