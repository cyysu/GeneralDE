#ifndef USF_BPG_CLI_PROXY_H
#define USF_BPG_CLI_PROXY_H
#include "cpe/cfg/cfg_types.h"
#include "bpg_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_cli_proxy_t
bpg_cli_proxy_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    bpg_pkg_manage_t pkg_manage,
    error_monitor_t em);

void bpg_cli_proxy_free(bpg_cli_proxy_t proxy);

gd_app_context_t bpg_cli_proxy_app(bpg_cli_proxy_t proxy);
const char * bpg_cli_proxy_name(bpg_cli_proxy_t proxy);

void bpg_cli_proxy_set_send_buf_capacity(bpg_cli_proxy_t proxy, size_t capacity);
int bpg_cli_proxy_set_send_to(bpg_cli_proxy_t proxy, cfg_t cfg);
int bpg_cli_proxy_set_recv_at(bpg_cli_proxy_t proxy, const char * name);

bpg_pkg_t bpg_cli_proxy_pkg_buf_for_send(bpg_cli_proxy_t proxy);

int bpg_cli_proxy_send(
    bpg_cli_proxy_t proxy,
    logic_require_t require,
    bpg_pkg_t pkg);

#ifdef __cplusplus
}
#endif

#endif
