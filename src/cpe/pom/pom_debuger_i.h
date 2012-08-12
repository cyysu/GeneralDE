#ifndef CPE_POM_DEBUGER_H
#define CPE_POM_DEBUGER_H
#include "cpe/utils/hash.h"
#include "cpe/utils/error.h"
#include "cpe/pom/pom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pom_alloc_info {
    pom_oid_t m_oid;
    int m_stack_size;
    int m_free;
    struct cpe_hash_entry m_hh;
};

struct pom_debuger {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint32_t m_stack_size;
    struct cpe_hash_table m_alloc_infos;
};

struct pom_debuger * pom_debuger_create(mem_allocrator_t alloc, uint32_t m_stack_size, error_monitor_t em);
void pom_debuger_free(struct pom_debuger * debuger);

void pom_debuger_on_alloc(struct pom_debuger * debuger, pom_oid_t oid);
void pom_debuger_on_free(struct pom_debuger * debuger, pom_oid_t oid);

#ifdef __cplusplus
}
#endif

#endif


