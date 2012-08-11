#ifndef USF_POM_GS_INTERNAL_OPS_H
#define USF_POM_GS_INTERNAL_OPS_H
#include "pom_gs_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void * pom_gs_agent_buf(pom_gs_agent_t agent, size_t capacity);

int pom_gs_pkg_buf_resize(pom_gs_pkg_t pkg, struct pom_gs_pkg_data_entry * data_entry, size_t capacity);

#define pom_gs_pkg_head_capacity(__count) \
    (sizeof(struct pom_gs_pkg) + (__count) * sizeof(struct pom_gs_pkg_data_entry))

#define pom_gs_pkg_entry_buf(__pkg, __entry) \
    (((char *)(__pkg)) + ((__pkg)->m_data_start) + ((__entry)->m_data_start))

#define pom_gs_pkg_mask_buf(__pkg, __entry) \
    (((char *)(__pkg)) + ((__entry)->m_mask_start))


#ifdef __cplusplus
}
#endif

#endif
