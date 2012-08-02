#ifndef USF_POM_GS_AGENT_H
#define USF_POM_GS_AGENT_H
#include "cpe/cfg/cfg_types.h"
#include "pom_gs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_gs_agent_t
pom_gs_agent_create(
    gd_app_context_t app,
    const char * name,
    pom_grp_meta_t pom_grp_meta,
    const char * main_entry,
    const char * key,
    mem_allocrator_t alloc,
    error_monitor_t em);

void pom_gs_agent_free(pom_gs_agent_t agent);

int pom_gs_agent_set_backend(pom_gs_agent_t agent, pom_gs_agent_backend_t backend, void * ctx);
int pom_gs_agent_remove_backend(pom_gs_agent_t agent);
pom_gs_agent_backend_t pom_gs_agent_backend(pom_gs_agent_t agent);

pom_gs_agent_t pom_gs_agent_find(gd_app_context_t app, cpe_hash_string_t name);
pom_gs_agent_t pom_gs_agent_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t pom_gs_agent_app(pom_gs_agent_t agent);
const char * pom_gs_agent_name(pom_gs_agent_t agent);

pom_gs_agent_t pom_gs_agent_insert(pom_gs_agent_t agent, pom_grp_obj_t obj, logic_require_t require);

#ifdef __cplusplus
}
#endif

#endif
