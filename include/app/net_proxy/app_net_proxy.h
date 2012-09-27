#ifndef APP_NET_PROXY_PROXY_H
#define APP_NET_PROXY_PROXY_H
#include "cpe/tl/tl_types.h"
#include "app_net_proxy_types.h"

#ifdef __cplusplus
extern "C" {
#endif

app_net_proxy_t
app_net_proxy_create(
    gd_app_context_t app,
    app_net_pkg_manage_t pkg_manage,
    const char * name,
    const char * ip,
    short port,
    size_t read_chanel_size,
    size_t write_chanel_size,
    mem_allocrator_t alloc,
    error_monitor_t em);

void app_net_proxy_free(app_net_proxy_t svr);

app_net_proxy_t
app_net_proxy_find(gd_app_context_t app, cpe_hash_string_t name);

app_net_proxy_t
app_net_proxy_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t app_net_proxy_app(app_net_proxy_t mgr);
const char * app_net_proxy_name(app_net_proxy_t mgr);
cpe_hash_string_t app_net_proxy_name_hs(app_net_proxy_t mgr);

void app_net_proxy_set_conn_timeout(app_net_proxy_t agent, tl_time_span_t span);
tl_time_span_t app_net_proxy_conn_timeout(app_net_proxy_t agent);

int app_net_proxy_set_dispatch_to(app_net_proxy_t agent, const char * dispatch_to);
short app_net_proxy_port(app_net_proxy_t svr);

#ifdef __cplusplus
}
#endif

#endif
