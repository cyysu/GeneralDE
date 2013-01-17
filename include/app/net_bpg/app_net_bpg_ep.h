#ifndef APP_NET_BPG_AGENT_TCP_H
#define APP_NET_BPG_AGENT_TCP_H
#include "cpe/tl/tl_types.h"
#include "usf/logic/logic_types.h"
#include "app_net_bpg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

app_net_bpg_ep_t
app_net_bpg_ep_create(
    gd_app_context_t app,
    bpg_pkg_manage_t pkg_manage,
    logic_manage_t logic_manage,
    const char * name,
    app_net_proxy_t app_net_proxy,
    uint16_t app_type,
    uint16_t app_id,
    mem_allocrator_t alloc,
    error_monitor_t em);

void app_net_bpg_ep_free(app_net_bpg_ep_t svr);

app_net_bpg_ep_t
app_net_bpg_ep_find(gd_app_context_t app, cpe_hash_string_t name);

app_net_bpg_ep_t
app_net_bpg_ep_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t app_net_bpg_ep_app(app_net_bpg_ep_t mgr);
const char * app_net_bpg_ep_name(app_net_bpg_ep_t mgr);
cpe_hash_string_t app_net_bpg_ep_name_hs(app_net_bpg_ep_t mgr);

int app_net_bpg_ep_set_dispatch_to(app_net_bpg_ep_t agent, const char * dispatch_to);

#ifdef __cplusplus
}
#endif

#endif
