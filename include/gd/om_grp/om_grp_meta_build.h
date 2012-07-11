#ifndef GD_OM_GRP_META_BUILD_H
#define GD_OM_GRP_META_BUILD_H
#include "cpe/cfg/cfg_types.h"
#include "om_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

om_grp_meta_t
om_grp_meta_build_from_cfg(mem_allocrator_t alloc, cfg_t cfg, LPDRMETALIB metalib, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
