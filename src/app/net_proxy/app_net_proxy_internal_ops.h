#ifndef APP_NET_PROXY_INTERNAL_OPS_H
#define APP_NET_PROXY_INTERNAL_OPS_H
#include "cpe/net/net_types.h"
#include "app_net_proxy_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*agent process*/
int app_net_proxy_ep_init(app_net_proxy_t proxy, net_ep_t ep, size_t read_chanel_size, size_t write_chanel_size);
app_net_pkg_t app_net_proxy_req_buf(app_net_proxy_t mgr);


/*ep operations*/
uint32_t app_net_ep_hash(const struct app_net_ep * ep);
int app_net_ep_eq(const struct app_net_ep * l, const struct app_net_ep * r);
void app_net_ep_clear_all(app_net_proxy_t proxy);
int app_net_ep_do_send(dp_req_t req, void * ctx, error_monitor_t em);

void app_net_ep_set_state(app_net_ep_t ep, app_net_ep_state_t state);

void app_net_ep_send_regist_req(app_net_ep_t ep, net_ep_t net_ep);

#ifdef __cplusplus
}
#endif

#endif
