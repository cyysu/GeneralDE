#ifndef GD_OM_GRP_OBJ_H
#define GD_OM_GRP_OBJ_H
#include "om_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

om_grp_obj_t
om_mgr_obj_alloc(om_grp_obj_mgr_t mgr);

void om_grp_obj_free(om_grp_obj_mgr_t mgr, om_grp_obj_t obj);

#ifdef __cplusplus
}
#endif

#endif
