#ifndef USF_MONGO_AGENT_H
#define USF_MONGO_AGENT_H
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "mongo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

mongo_agent_t
mongo_agent_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    mem_allocrator_t alloc,
    error_monitor_t em);

void mongo_agent_free(mongo_agent_t agent);
int mongo_agent_build_reulst_metalib(mongo_agent_t agent);

mongo_agent_t
mongo_agent_find(gd_app_context_t app, cpe_hash_string_t name);

mongo_agent_t
mongo_agent_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t mongo_agent_app(mongo_agent_t agent);
const char * mongo_agent_name(mongo_agent_t agent);
cpe_hash_string_t mongo_agent_name_hs(mongo_agent_t agent);

mongo_agent_state_t mongo_agent_state(mongo_agent_t agent);

int mongo_agent_add_seed(mongo_agent_t agent, const char * host, int port);
int mongo_agent_add_server(mongo_agent_t agent, const char * host, int port);

int mongo_agent_enable(mongo_agent_t agent);

int mongo_agent_send_request(mongo_agent_t agent, mongo_request_t request, logic_require_t require);

#ifdef __cplusplus
}
#endif

#endif
