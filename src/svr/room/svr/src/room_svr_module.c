#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_stub.h"
#include "room_svr_ops.h"

EXPORT_DIRECTIVE
int room_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    room_svr_t room_svr;
    uint32_t check_span_ms;
    uint32_t timeout_span_s;
    const char * send_to;
    const char * recv_at;

    if ((send_to = cfg_get_string(cfg, "send-to", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    if ((recv_at = cfg_get_string(cfg, "recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cfg_try_get_uint32(cfg, "check-span-ms", &check_span_ms) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: check-span-ms not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cfg_try_get_uint32(cfg, "timeout-span-s", &timeout_span_s) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: timeout-span-s not configured!", gd_app_module_name(module));
        return -1;
    }

    room_svr =
        room_svr_create(
            app, gd_app_module_name(module),
            gd_app_alloc(app), gd_app_em(app));
    if (room_svr == NULL) return -1;

    room_svr->m_debug = cfg_get_int8(cfg, "debug", room_svr->m_debug);

    if (room_svr_set_send_to(room_svr, send_to) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set send-to %s fail!", gd_app_module_name(module), send_to);
        return -1;
    }

    if (room_svr_set_recv_at(room_svr, recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set recv-at %s fail!", gd_app_module_name(module), recv_at);
        return -1;
    }

    if (room_svr_set_check_span(room_svr, check_span_ms) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set check-span-ms %d fail!", gd_app_module_name(module), check_span_ms);
        room_svr_free(room_svr);
        return -1;
    }

    room_svr->m_timeout_span_s = timeout_span_s;

    if (room_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void room_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    room_svr_t room_svr;

    room_svr = room_svr_find_nc(app, gd_app_module_name(module));
    if (room_svr) {
        room_svr_free(room_svr);
    }
}
