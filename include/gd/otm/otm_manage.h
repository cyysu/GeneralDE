#ifndef GD_OTM_MANAGER_H
#define GD_OTM_MANAGER_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "otm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

otm_manage_t
otm_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void otm_manage_free(otm_manage_t mgr);

otm_manage_t
otm_manage_find(gd_app_context_t app, cpe_hash_string_t name);

otm_manage_t
otm_manage_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t otm_manage_app(otm_manage_t mgr);
const char * otm_manage_name(otm_manage_t mgr);
cpe_hash_string_t otm_manage_name_hs(otm_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif
