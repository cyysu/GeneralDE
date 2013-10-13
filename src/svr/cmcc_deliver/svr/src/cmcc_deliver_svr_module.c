#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/net_trans/net_trans_manage.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "cmcc_deliver_svr_ops.h"

EXPORT_DIRECTIVE
int cmcc_deliver_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    cmcc_deliver_svr_t cmcc_deliver_svr;
    const char * send_to;
    const char * request_recv_at;

    if ((send_to = cfg_get_string(cfg, "send-to", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    if ((request_recv_at = cfg_get_string(cfg, "request-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: request-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    cmcc_deliver_svr =
        cmcc_deliver_svr_create(
            app, gd_app_module_name(module),
            stub, gd_app_alloc(app), gd_app_em(app));
    if (cmcc_deliver_svr == NULL) return -1;

    cmcc_deliver_svr->m_debug = cfg_get_int8(cfg, "debug", cmcc_deliver_svr->m_debug);

    if (cmcc_deliver_svr_set_send_to(cmcc_deliver_svr, send_to) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set send-to %s fail!", gd_app_module_name(module), send_to);
        cmcc_deliver_svr_free(cmcc_deliver_svr);
        return -1;
    }

    if (cmcc_deliver_svr_set_request_recv_at(cmcc_deliver_svr, request_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set request-recv-at %s fail!", gd_app_module_name(module), request_recv_at);
        cmcc_deliver_svr_free(cmcc_deliver_svr);
        return -1;
    }

    if (cmcc_deliver_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void cmcc_deliver_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    cmcc_deliver_svr_t cmcc_deliver_svr;

    cmcc_deliver_svr = cmcc_deliver_svr_find_nc(app, gd_app_module_name(module));
    if (cmcc_deliver_svr) {
        cmcc_deliver_svr_free(cmcc_deliver_svr);
    }
}

