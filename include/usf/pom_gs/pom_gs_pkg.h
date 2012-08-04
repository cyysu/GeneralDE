#ifndef USF_POM_GS_DATA_H
#define USF_POM_GS_DATA_H
#include "cpe/cfg/cfg_types.h"
#include "pom_gs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern cpe_hash_string_t pom_gs_pkg_type_name;

pom_gs_pkg_t
pom_gs_pkg_create(pom_gs_agent_t agent, size_t capacity);
void pom_gs_pkg_free(pom_gs_pkg_t pkg);

pom_gs_agent_t pom_gs_pkg_agent(pom_gs_pkg_t pkg);

void pom_gs_pkg_init(pom_gs_pkg_t pkg);

void * pom_gs_pkg_buf(pom_gs_pkg_t pkg, const char * table_name, size_t capacity);
size_t pom_gs_pkg_buf_capacity(pom_gs_pkg_t pkg, const char * table_name);

#ifdef __cplusplus
}
#endif

#endif
