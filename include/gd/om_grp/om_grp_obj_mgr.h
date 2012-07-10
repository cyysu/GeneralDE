#ifndef GD_OM_GRP_MANAGE_H
#define GD_OM_GRP_MANAGE_H
#include "cpe/dr/dr_types.h"
#include "om_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

om_grp_obj_mgr_t
om_grp_obj_mgr_create(
    mem_allocrator_t alloc,
    void * data,
    size_t data_capacity,
    error_monitor_t em);

void om_grp_obj_mgr_free(om_grp_obj_mgr_t mgr);

gd_om_mgr_t om_grp_obj_mgr_omm(om_grp_obj_mgr_t mgr);

int om_grp_obj_mgr_buf_init(
    LPDRMETA metalib,
    om_grp_meta_t grp_meta,
    uint16_t page_size,
    uint16_t buffer_size,
    void * data, size_t data_capacity,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
