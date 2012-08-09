#ifndef USF_POM_GS_PKG_H
#define USF_POM_GS_PKG_H
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

int pom_gs_pkg_table_buf_clear(pom_gs_pkg_t pkg, const char * table_name);
void * pom_gs_pkg_table_buf(pom_gs_pkg_t pkg, const char * table_name, size_t * capacity);
size_t pom_gs_pkg_table_buf_capacity(pom_gs_pkg_t pkg, const char * table_name);

int pom_gs_pkg_set_entry(
    pom_gs_pkg_t pkg, const char * entry_name,
    LPDRMETA meta, void const * data, size_t data_size);

int pom_gs_pkg_set_entry_part(
    pom_gs_pkg_t pkg, const char * entry_name,
    LPDRMETA meta, void const * data, size_t data_size,
    const char * sub_entries);

#ifdef __cplusplus
}
#endif

#endif
