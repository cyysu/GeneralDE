#ifndef APP_NET_PKG_INTERNAL_TYPES_H
#define APP_NET_PKG_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "cpe/net/net_types.h"
#include "app/net_pkg/app_net_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_net_pkg_manage {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
};

struct app_net_pkg {
    app_net_pkg_manage_t m_mgr;
    dp_req_t m_dp_req;
};

#ifdef __cplusplus
}
#endif

#endif
