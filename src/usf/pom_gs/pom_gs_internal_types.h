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

struct pom_gs_pkg_data_entry {
    pom_grp_store_table_t m_table;
    uint32_t m_start;
    uint32_t m_capacity;
};

struct pom_gs_pkg {
    pom_gs_agent_t m_agent;
    dp_req_t m_dp_req;
    uint32_t m_entry_count;
    struct pom_gs_pkg_data_entry m_entries[0];
};

#ifdef __cplusplus
}
#endif

#endif
