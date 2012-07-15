#ifndef GD_POM_MGR_INTERNAL_TYPES_H
#define GD_POM_MGR_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "gd/pom_mgr/pom_mgr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pom_manage {
    mem_allocrator_t m_alloc;
    gd_app_context_t m_app;
    mem_allocrator_t m_allc;
    error_monitor_t m_em;

    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
