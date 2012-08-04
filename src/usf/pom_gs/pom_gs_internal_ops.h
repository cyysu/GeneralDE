#ifndef USF_POM_GS_INTERNAL_OPS_H
#define USF_POM_GS_INTERNAL_OPS_H
#include "pom_gs_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void * pom_gs_agent_buf(pom_gs_agent_t agent, size_t capacity);

#define pom_gs_pkg_head_capacity(__count) \
    (sizeof(struct pom_gs_pkg) - sizeof(struct pom_gs_pkg_data_entry) + (__count) * sizeof(struct pom_gs_pkg_data_entry))

#define pom_gs_pkg_entry_buf(__pkg, __entry) \
    (((char *)(__pkg)) + pom_gs_pkg_head_capacity(((__pkg)->m_entry_count)) + ((__entry)->m_start))


#ifdef __cplusplus
}
#endif

#endif
