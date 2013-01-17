#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "app/net_pkg/app_net_pkg.h"
#include "app/net_pkg/app_net_pkg_manage.h"
#include "app_net_pkg_internal_types.h"

EXPORT_DIRECTIVE
int app_net_pkg_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    app_net_pkg_manage_t app_net_pkg_manage;

    app_net_pkg_manage = app_net_pkg_manage_create(app, gd_app_module_name(module), gd_app_em(app));
    if (app_net_pkg_manage == NULL) {
        return -1;
    }

    app_net_pkg_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (app_net_pkg_manage->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void app_net_pkg_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    app_net_pkg_manage_t app_net_pkg_manage;

    app_net_pkg_manage = app_net_pkg_manage_find_nc(app, gd_app_module_name(module));
    if (app_net_pkg_manage) {
        app_net_pkg_manage_free(app_net_pkg_manage);
    }
}
