#ifndef APP_NET_BPG_INTERNAL_OPS_H
#define APP_NET_BPG_INTERNAL_OPS_H
#include "app_net_bpg_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*agent process*/
void app_net_bpg_ep_accept(net_listener_t listener, net_ep_t ep, void * ctx);
int app_net_bpg_ep_reply(dp_req_t req, void * ctx, error_monitor_t em);
bpg_pkg_t app_net_bpg_ep_req_buf(app_net_bpg_ep_t mgr);
int app_net_bpg_ep_notify_client(dp_req_t req, void * ctx, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
