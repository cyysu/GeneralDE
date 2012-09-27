#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/net/net_connector.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "app/net_pkg/app_net_pkg.h"
#include "app/net_pkg/app_net_pkg_manage.h"
#include "app/net_proxy/app_net_proxy.h"
#include "app_net_proxy_internal_ops.h"

EXPORT_DIRECTIVE
int app_net_proxy_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    app_net_proxy_t app_net_proxy;
    const char * ip;
    short port;
    app_net_pkg_manage_t pkg_manage;
    cfg_t req_recv_cfg;

    pkg_manage = app_net_pkg_manage_find_nc(app, cfg_get_string(cfg, "pkg-manage", NULL));
    if (pkg_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: pkg-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "pkg-manage", "default"));
        return -1;
    }

    req_recv_cfg = cfg_find_cfg(cfg, "req-recv-at");
    if (req_recv_cfg == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: req-recv-at not configured!",
            gd_app_module_name(module));
        return -1;
    }

    ip = cfg_get_string(cfg, "ip", "");
    port = cfg_get_int16(cfg, "port", 0);

    app_net_proxy =
        app_net_proxy_create(
            app, pkg_manage, gd_app_module_name(module),
            ip, port,
            cfg_get_uint32(cfg, "read-chanel-size", 2 * 1024),
            cfg_get_uint32(cfg, "write-chanel-size", 2 * 1024),
            gd_app_alloc(app), gd_app_em(app));
    if (app_net_proxy == NULL) return -1;

    app_net_proxy->m_req_max_size =
        cfg_get_uint32(cfg, "req-max-size", app_net_proxy->m_req_max_size);

    app_net_proxy->m_debug = cfg_get_int8(cfg, "debug", app_net_proxy->m_debug);

    if (app_net_proxy->m_debug) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: done. ip=%s, port=%u, req-max-size=%d",
            gd_app_module_name(module),
            ip, port, 
            (int)app_net_proxy->m_req_max_size);
    }

    if (cfg_get_int32(cfg, "auto-enable", 0)) {
        if (net_connector_enable(app_net_proxy->m_connector) == 0) {
            if (app_net_proxy->m_debug) {
                CPE_INFO(
                    gd_app_em(app),
                    "%s: create: auto-enable success!",
                    gd_app_module_name(module));
            }
        }
        else {
            CPE_ERROR(
                gd_app_em(app),
                "%s: create: auto-enable fail!",
                gd_app_module_name(module));
            app_net_proxy_free(app_net_proxy);
            return -1;
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
void app_net_proxy_app_fini(gd_app_context_t app, gd_app_module_t module) {
    app_net_proxy_t app_net_proxy;

    app_net_proxy = app_net_proxy_find_nc(app, gd_app_module_name(module));
    if (app_net_proxy) {
        app_net_proxy_free(app_net_proxy);
    }
}
