#ifndef APP_NET_PROXY_EP_H
#define APP_NET_PROXY_EP_H
#include "app_net_proxy_types.h"

#ifdef __cplusplus
extern "C" {
#endif

app_net_ep_t
app_net_ep_create(app_net_proxy_t proxy, uint16_t app_type, uint16_t app_id);

void app_net_ep_free(app_net_ep_t ep);

uint16_t app_net_ep_app_type(app_net_ep_t ep);
uint16_t app_net_ep_app_id(app_net_ep_t ep);

int app_net_ep_set_outgoing_recv_at(app_net_ep_t ep, const char * outgoing_recv_at);
int app_net_ep_set_incoming_send_to(app_net_ep_t ep, const char * incoming_send_to);

app_net_ep_t app_net_ep_find(app_net_proxy_t proxy, uint16_t app_type);

int app_net_ep_send(app_net_ep_t ep, app_net_pkg_t pkg);

#ifdef __cplusplus
}
#endif

#endif

