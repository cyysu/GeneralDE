#ifndef USF_BPG_USE_PROXY_H
#define USF_BPG_USE_PROXY_H
#include "cpe/cfg/cfg_types.h"
#include "bpg_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_use_proxy_t
bpg_use_proxy_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    error_monitor_t em);

void bpg_use_proxy_free(bpg_use_proxy_t proxy);

gd_app_context_t bpg_use_proxy_app(bpg_use_proxy_t proxy);
const char * bpg_use_proxy_name(bpg_use_proxy_t proxy);

#ifdef __cplusplus
}
#endif

#endif
