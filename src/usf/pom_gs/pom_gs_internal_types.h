#ifndef USF_POM_GS_INTERNAL_TYPES_H
#define USF_POM_GS_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "usf/pom_gs/pom_gs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pom_gs_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    pom_grp_store_t m_pom_grp_store;

    pom_gs_agent_backend_t m_backend;
    void * m_backend_ctx;

    pom_grp_meta_t m_pom_grp_meta;
    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
