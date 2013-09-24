#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/net_trans/net_trans_manage.h"
#include "net_trans_internal_ops.h"

EXPORT_DIRECTIVE
int net_trans_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    net_trans_manage_t net_trans_manage;

    net_trans_manage =
        net_trans_manage_create(
            app, gd_app_module_name(module),
            gd_app_alloc(app), gd_app_em(app));
    if (net_trans_manage == NULL) return -1;

    net_trans_manage->m_debug = cfg_get_int8(cfg, "debug", net_trans_manage->m_debug);

    if (net_trans_mult_handler_init(net_trans_manage) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: init mult handler fail!", gd_app_module_name(module));
        net_trans_manage_free(net_trans_manage);
        return -1;
    }

    if (net_trans_manage->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void net_trans_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    net_trans_manage_t net_trans_manage;

    net_trans_manage = net_trans_manage_find_nc(app, gd_app_module_name(module));
    if (net_trans_manage) {
        net_trans_manage_free(net_trans_manage);
    }
}

