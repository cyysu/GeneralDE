#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/om/om_manage.h"
#include "gd/om_grp/om_grp_obj_mgr.h"
#include "gd/om_grp/om_grp_meta.h"
#include "om_grp_internal_ops.h"

gd_om_buffer_id_t om_grp_backend_alloc(size_t size, void * context) {
    ptr_int_t id;
    om_grp_obj_mgr_t mgr = (om_grp_obj_mgr_t)context;

    if (cpe_range_mgr_is_empty(&mgr->m_alloc_range)) {
        CPE_ERROR(mgr->m_em, "om_grp_backend_alloc: no page left!");
        return GD_OM_INVALID_BUFFER_ID;
    }

    id = (gd_om_buffer_id_t)cpe_range_get_one(&mgr->m_alloc_range);
    cpe_ba_set(mgr->m_alloc_ba, id, cpe_ba_true);

    return (gd_om_buffer_id_t)(mgr->m_data_base + mgr->m_meta->m_omm_buffer_size * id);
}

void om_grp_backend_clear(struct gd_om_buffer_id_it * buf_ids, void * context) {
}

struct gd_om_backend g_om_grp_backend = {
    om_grp_backend_alloc
    , NULL
    , om_grp_backend_clear
};
