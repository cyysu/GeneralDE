#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "match_svr_ops.h"

static int match_svr_app_load_room_and_user_data(match_svr_t match_svr, cfg_t cfg);

EXPORT_DIRECTIVE
int match_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    set_svr_svr_info_t room_svr_type;
    match_svr_t match_svr;
    uint32_t check_span_ms;
    uint32_t create_retry_span_s;
    const char * send_to;
    const char * match_request_recv_at;

    if ((send_to = cfg_get_string(cfg, "send-to", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    if ((match_request_recv_at = cfg_get_string(cfg, "match-request-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: match-request-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cfg_try_get_uint32(cfg, "check-span-ms", &check_span_ms) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: check-span-ms not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cfg_try_get_uint32(cfg, "create-retry-span-s", &create_retry_span_s) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: create-retry-span-s not configured!", gd_app_module_name(module));
        return -1;
    }

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    room_svr_type = set_svr_svr_info_find_by_name(stub, "svr_room");
    if (room_svr_type == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: svr_room find type fail!", gd_app_module_name(module));
        return -1;
    }

    match_svr =
        match_svr_create(
            app, gd_app_module_name(module),
            stub, set_svr_svr_info_svr_type_id(room_svr_type),
            gd_app_alloc(app), gd_app_em(app));
    if (match_svr == NULL) return -1;

    match_svr->m_debug = cfg_get_int8(cfg, "debug", match_svr->m_debug);

    if (match_svr_set_send_to(match_svr, send_to) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set send-to %s fail!", gd_app_module_name(module), send_to);
        match_svr_free(match_svr);
        return -1;
    }

    if (match_svr_set_match_require_recv_at(match_svr, match_request_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set match-request-recv-at %s fail!", gd_app_module_name(module), match_request_recv_at);
        match_svr_free(match_svr);
        return -1;
    }

    if (match_svr_set_check_span(match_svr, check_span_ms) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set check-span-ms %d fail!", gd_app_module_name(module), check_span_ms);
        match_svr_free(match_svr);
        return -1;
    }

    match_svr->m_create_retry_span_s = create_retry_span_s;

    if (match_svr_meta_room_load(match_svr, cfg_find_cfg(gd_app_cfg(app), "meta.match_room_def")) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load match_room_def fail!", gd_app_module_name(module));
        match_svr_free(match_svr);
        return -1;
    }

    if (match_svr_app_load_room_and_user_data(match_svr, cfg) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load room and user data fail!", gd_app_module_name(module));
        match_svr_free(match_svr);
        return -1;
    }

    if (match_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void match_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    match_svr_t match_svr;

    match_svr = match_svr_find_nc(app, gd_app_module_name(module));
    if (match_svr) {
        match_svr_free(match_svr);
    }
}

int match_svr_app_load_room_and_user_data(match_svr_t svr, cfg_t cfg) {
    const char * str_room_buf_size = cfg_get_string(cfg, "room-buf-size", NULL);
    const char * str_user_buf_size = cfg_get_string(cfg, "user-buf-size", NULL);
    uint64_t room_buf_size;
    uint64_t user_buf_size;

    if (str_room_buf_size == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: room-buf-size not configured!", match_svr_name(svr));
        return -1;
    }

    if (cpe_str_parse_byte_size(&room_buf_size, str_room_buf_size) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: room-buf-size %s format error!", match_svr_name(svr), str_room_buf_size);
        return -1;
    }

    if (str_user_buf_size == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: user-buf-size not configured!", match_svr_name(svr));
        return -1;
    }

    if (cpe_str_parse_byte_size(&user_buf_size, str_user_buf_size) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: user-buf-size %s format error!", match_svr_name(svr), str_user_buf_size);
        return -1;
    }
    
    if (cfg_get_int32(cfg, "load-from-shm", 0)) {
        CPE_ERROR(svr->m_em, "%s: create: not support load room and user data from shm!", match_svr_name(svr));
        return -1;
    }
    else {
        if (match_svr_room_data_init_from_mem(svr, room_buf_size) != 0) return -1;
        if (match_svr_user_data_init_from_mem(svr, user_buf_size) != 0) return -1;
    }

    return 0;
}
