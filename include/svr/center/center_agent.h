#ifndef SVR_CENTER_AGENT_H
#define SVR_CENTER_AGENT_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "center_agent_types.h"

#ifdef __cplusplus
extern "C" {
#endif

center_agent_t center_agent_create(
    gd_app_context_t app,
    const char * name, 
    mem_allocrator_t alloc, error_monitor_t em);

void center_agent_free(center_agent_t mgr);

int center_agent_set_svr(center_agent_t agent, const char * ip, short port);
int center_agent_set_cvt(center_agent_t agent, const char * cvt_name);
int center_agent_set_reconnect_span_ms(center_agent_t agent, uint32_t span);

center_agent_t center_agent_find(gd_app_context_t app, cpe_hash_string_t name);
center_agent_t center_agent_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t center_agent_app(center_agent_t mgr);
const char * center_agent_name(center_agent_t mgr);
cpe_hash_string_t center_agent_name_hs(center_agent_t mgr);

#ifdef __cplusplus
}
#endif

#endif