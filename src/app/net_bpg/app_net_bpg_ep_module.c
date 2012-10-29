#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/logic/logic_manage.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "app/net_proxy/app_net_proxy.h"
#include "app/net_bpg/app_net_bpg_ep.h"
#include "app_net_bpg_internal_ops.h"

EXPORT_DIRECTIVE
int app_net_bpg_ep_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    app_net_bpg_ep_t app_net_bpg_ep;
    bpg_pkg_manage_t pkg_manage;
    logic_manage_t logic_manage;
    app_net_proxy_t app_net_proxy;
    cfg_t reply_recv_cfg;
    uint16_t app_type;
    uint16_t app_id;

    pkg_manage = bpg_pkg_manage_find_nc(app, cfg_get_string(cfg, "pkg-manage", NULL));
    if (pkg_manage == NULL) {
        CPE_ERROR(
                gd_app_em(app), "%s: create: pkg-manage %s not exist!",
                gd_app_module_name(module),
                cfg_get_string(cfg, "pkg-manage", "default"));
        return -1;
    }

    logic_manage = logic_manage_find_nc(app, cfg_get_string(cfg, "logic-manage", NULL));
    if (logic_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: logic-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "logic-manage", "default"));
        return -1;
    }

    app_net_proxy = app_net_proxy_find_nc(app, cfg_get_string(cfg, "app-net", NULL));
    if (app_net_proxy == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: app-net-proxy %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "app-net", "default"));
        return -1;
    }

    app_type = cfg_get_uint16(cfg, "app-type", 0);
    if (app_type == 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: app-net-proxy %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "app-net", "default"));
    }

    app_id = 0;

    reply_recv_cfg = cfg_find_cfg(cfg, "reply-recv-at");
    if (reply_recv_cfg == NULL) {
        CPE_ERROR(
                gd_app_em(app), "%s: create: reply-recv-at not configured!",
                gd_app_module_name(module));
        return -1;
    }

    app_net_bpg_ep =
        app_net_bpg_ep_create(
            app, pkg_manage, logic_manage, gd_app_module_name(module),
            app_net_proxy, app_type, app_id,
            gd_app_alloc(app), gd_app_em(app));
    if (app_net_bpg_ep == NULL) return -1;

    app_net_bpg_ep->m_req_max_size =
        cfg_get_uint32(cfg, "req-max-size", app_net_bpg_ep->m_req_max_size);

    app_net_bpg_ep->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (dp_rsp_bind_by_cfg(app_net_bpg_ep->m_reply_rsp, reply_recv_cfg, gd_app_em(app)) != 0) {
        CPE_ERROR(
                gd_app_em(app), "%s: create: bind rsp by cfg fail!",
                gd_app_module_name(module));
        app_net_bpg_ep_free(app_net_bpg_ep);
        return -1;
    }

    if (app_net_bpg_ep_set_dispatch_to(app_net_bpg_ep, cfg_get_string(cfg, "dispatch-to", NULL)) != 0) {
        CPE_ERROR(
                gd_app_em(app), "%s: create: set dispatch to fail!",
                gd_app_module_name(module));
        app_net_bpg_ep_free(app_net_bpg_ep);
        return -1;
    }

    if (app_net_bpg_ep->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void app_net_bpg_ep_app_fini(gd_app_context_t app, gd_app_module_t module) {
    app_net_bpg_ep_t app_net_bpg_ep;

    app_net_bpg_ep = app_net_bpg_ep_find_nc(app, gd_app_module_name(module));
    if (app_net_bpg_ep) {
        app_net_bpg_ep_free(app_net_bpg_ep);
    }
}
