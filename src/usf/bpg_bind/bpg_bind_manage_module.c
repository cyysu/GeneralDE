#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "usf/bpg_bind/bpg_bind_manage.h"
#include "bpg_bind_internal_types.h"

EXPORT_DIRECTIVE
int bpg_bind_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    bpg_bind_manage_t bpg_bind_manage;
    bpg_pkg_manage_t pkg_manage;
    const char * recv_at;
	const char * reply_to;

	pkg_manage = bpg_pkg_manage_find_nc(app, cfg_get_string(cfg, "pkg-manage", NULL));
	if (pkg_manage == NULL) {
		CPE_ERROR(
			gd_app_em(app), "%s: create: pkg-manage %s not exist!",
			gd_app_module_name(module),
			cfg_get_string(cfg, "pkg-manage", "default"));
		return -1;
	}

    recv_at = cfg_get_string(cfg, "recv-at", NULL);
    if (recv_at == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: recv-at not configured!",
            gd_app_module_name(module));
        return -1;
    }

	reply_to = cfg_get_string(cfg, "reply-to", NULL);
	if (reply_to == NULL) {
		CPE_ERROR(
			gd_app_em(app), "%s: create: reply_to not configured!",
			gd_app_module_name(module));
		return -1;
	}

    bpg_bind_manage =
        bpg_bind_manage_create(
            app,
            gd_app_alloc(app),
            gd_app_module_name(module),
            pkg_manage,
            gd_app_em(app));
    if (bpg_bind_manage == NULL) return -1;

    if (bpg_bind_manage_set_recv_at(bpg_bind_manage, recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set recv-at %s fail!",
            gd_app_module_name(module),
            recv_at);
        bpg_bind_manage_free(bpg_bind_manage);
        return -1;
    }

	if (bpg_bind_manage_set_reply_to(bpg_bind_manage, reply_to) != 0) {
		CPE_ERROR(
			gd_app_em(app), "%s: create: set recv-at %s fail!",
			gd_app_module_name(module),
			recv_at);
		bpg_bind_manage_free(bpg_bind_manage);
		return -1;
	}

    bpg_bind_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (bpg_bind_manage->m_debug) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: done., rsp-manage=%s, recv-at=%s",
            gd_app_module_name(module),
            bpg_pkg_manage_name(pkg_manage),
            recv_at);
    }

    return 0;
}

EXPORT_DIRECTIVE
void bpg_bind_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_bind_manage_t bpg_bind_manage;

    bpg_bind_manage = bpg_bind_manage_find_nc(app, gd_app_module_name(module));
    if (bpg_bind_manage) {
        bpg_bind_manage_free(bpg_bind_manage);
    }
}
