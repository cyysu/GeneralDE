#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "gd/om/om_object.h"
#include "gd/om/om_manage.h"
#include "gd/om_grp/om_grp_obj.h"
#include "om_grp_internal_ops.h"

om_grp_obj_t
om_mgr_obj_alloc(om_grp_obj_mgr_t mgr) {
    gd_om_oid_t control_oid;
    om_grp_obj_t obj;
    assert(mgr);

    control_oid =
        gd_om_obj_alloc(mgr->m_omm, om_grp_control_class_name, mgr->m_em);
    if (control_oid == GD_OM_INVALID_OID) {
        CPE_ERROR(mgr->m_em, "om_mgr_obj_alloc: alloc control buf fail!");
        return NULL;
    }

    obj = (om_grp_obj_t)gd_om_obj_get(mgr->m_omm, control_oid, mgr->m_em);
    if (obj == NULL) {
        CPE_ERROR(mgr->m_em, "om_mgr_obj_alloc: get control buf fail!");
        return NULL;
    }

    return obj;
}

void om_grp_obj_free(om_grp_obj_mgr_t mgr, om_grp_obj_t obj) {
}



