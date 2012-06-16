#ifndef CPE_OTM_MANAGER_H
#define CPE_OTM_MANAGER_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "otm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

otm_manage_t
otm_manage_create(
    mem_allocrator_t alloc,
    error_monitor_t em);

void otm_manage_free(otm_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif
