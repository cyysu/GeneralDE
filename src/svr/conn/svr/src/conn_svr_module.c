#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/center/agent/center_agent.h"
#include "svr/center/agent/center_agent_svr_type.h"
#include "conn_svr_ops.h"

static int conn_svr_load_backends(conn_svr_t conn_svr);

EXPORT_DIRECTIVE
int conn_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    center_agent_t agent;
    conn_svr_t conn_svr;
    center_agent_svr_type_t conn_svr_type;
    uint32_t check_span_ms;
    uint32_t conn_timeout_s;
    const char * ip;
    short port;
    int accept_queue_size;
    const char * send_to;
    const char * ss_request_recv_at;
    const char * ss_response_recv_at;
    const char * ss_notify_recv_at;
    const char * str_ringbuf_size;
    uint64_t ringbuf_size;

    ip = cfg_get_string(cfg, "ip", "");
    port = cfg_get_int16(cfg, "port", 0);
    accept_queue_size = cfg_get_int32(cfg, "accept-queue-size", 256);

    if ((str_ringbuf_size = cfg_get_string(cfg, "ringbuf-size", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: ringbuf-size not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cpe_str_parse_byte_size(&ringbuf_size, str_ringbuf_size) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: read ringbuf-size %s fail!",
            gd_app_module_name(module), str_ringbuf_size);
        return -1;
    }

    if ((send_to = cfg_get_string(cfg, "ss-send-to", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: ss-send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    if ((ss_request_recv_at = cfg_get_string(cfg, "ss-request-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: ss-request-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    if ((ss_response_recv_at = cfg_get_string(cfg, "ss-response-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: ss-response-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    if ((ss_notify_recv_at = cfg_get_string(cfg, "ss-notify-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: ss-notify-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cfg_try_get_uint32(cfg, "check-span-ms", &check_span_ms) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: check-span-ms not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cfg_try_get_uint32(cfg, "conn-timeout-s", &conn_timeout_s) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: conn-timeout-s not configured!", gd_app_module_name(module));
        return -1;
    }

    agent = center_agent_find_nc(app, cfg_get_string(cfg, "center-agent", NULL));
    if (agent == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: center-agent %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "center-agent", "default"));
        return -1;
    }

    conn_svr_type = center_agent_svr_type_lsearch_by_name(agent, "svr_conn");
    if (conn_svr_type == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: svr_conn find type fail!", gd_app_module_name(module));
        return -1;
    }

    conn_svr =
        conn_svr_create(
            app, gd_app_module_name(module),
            agent,
            center_agent_svr_type_id(conn_svr_type),
            gd_app_alloc(app), gd_app_em(app));
    if (conn_svr == NULL) return -1;

    conn_svr->m_debug = cfg_get_int8(cfg, "debug", conn_svr->m_debug);

    if (conn_svr_set_ringbuf_size(conn_svr, ringbuf_size) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set ringbuf-size %d fail!", gd_app_module_name(module), (int)ringbuf_size);
        conn_svr_free(conn_svr);
        return -1;
    }

    if (conn_svr_set_ss_send_to(conn_svr, send_to) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set send-to %s fail!", gd_app_module_name(module), send_to);
        conn_svr_free(conn_svr);
        return -1;
    }

    if (conn_svr_set_ss_request_recv_at(conn_svr, ss_request_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set ss-request-recv-at %s fail!", gd_app_module_name(module), ss_request_recv_at);
        conn_svr_free(conn_svr);
        return -1;
    }

    if (conn_svr_set_ss_response_recv_at(conn_svr, ss_response_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set ss-response-recv-at %s fail!", gd_app_module_name(module), ss_response_recv_at);
        conn_svr_free(conn_svr);
        return -1;
    }

    if (conn_svr_set_ss_notify_recv_at(conn_svr, ss_notify_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set ss-notify-recv-at %s fail!", gd_app_module_name(module), ss_notify_recv_at);
        conn_svr_free(conn_svr);
        return -1;
    }

    if (conn_svr_set_check_span(conn_svr, check_span_ms) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set check-span-ms %d fail!", gd_app_module_name(module), check_span_ms);
        conn_svr_free(conn_svr);
        return -1;
    }

    if (conn_svr_start(conn_svr, ip, port, accept_queue_size) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: start at %s:%d fail fail!", gd_app_module_name(module), ip, port);
        conn_svr_free(conn_svr);
        return -1;
    }

    conn_svr->m_conn_timeout_s = conn_timeout_s;

    if (conn_svr_load_backends(conn_svr) != 0) {
        conn_svr_free(conn_svr);
        return -1;
    }

    if (conn_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done. listen at %s:%d", gd_app_module_name(module), ip, port);
    }

    return 0;
}

EXPORT_DIRECTIVE
void conn_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    conn_svr_t conn_svr;

    conn_svr = conn_svr_find_nc(app, gd_app_module_name(module));
    if (conn_svr) {
        conn_svr_free(conn_svr);
    }
}

static int conn_svr_init_backend(conn_svr_t conn_svr, center_agent_svr_type_t svr_type, cfg_t cfg) {
    conn_svr_backend_t backend;
    const char * str_safe_policy;

    backend = conn_svr_backend_create(conn_svr, center_agent_svr_type_id(svr_type));
    if (backend == NULL) {
        CPE_ERROR(conn_svr->m_em, "%s: create: backend %s fail!", conn_svr_name(conn_svr), center_agent_svr_type_name(svr_type));
        return -1;
    }

    str_safe_policy = cfg_get_string(cfg, "public.safe-policy", NULL);
    if (str_safe_policy == NULL) {
        CPE_ERROR(
            conn_svr->m_em, "%s: create: backend %s: safe-policy not configured!",
            conn_svr_name(conn_svr), center_agent_svr_type_name(svr_type));
        conn_svr_backend_free(backend);
        return -1;
    }

    if (strcmp(str_safe_policy, "any") == 0) {
        backend->m_safe_policy = conn_svr_backend_any;
    }
    else if (strcmp(str_safe_policy, "auth-success") == 0) {
        backend->m_safe_policy = conn_svr_backend_auth_success;
    }
    else if (strcmp(str_safe_policy, "user-bind") == 0) {
        backend->m_safe_policy = conn_svr_backend_user_bind;
    }
    else {
        CPE_ERROR(
            conn_svr->m_em, "%s: create: backend %s: safe-policy %s is unknown!",
            conn_svr_name(conn_svr), center_agent_svr_type_name(svr_type), str_safe_policy);
        conn_svr_backend_free(backend);
        return -1;
    }

    return 0;
}

static int conn_svr_load_backends(conn_svr_t conn_svr) {
    gd_app_context_t app = conn_svr->m_app;
    cfg_t svr_cfg = cfg_find_cfg(cfg_find_cfg(gd_app_cfg(app), "svr_types"), "svr_conn");
    struct cfg_it depends_it;
    cfg_t depend_cfg;

    if (svr_cfg == NULL) {
        CPE_ERROR(conn_svr->m_em, "%s: create: svr_type svr_conn not configured!", conn_svr_name(conn_svr));
        return -1;
    }

    cfg_it_init(&depends_it, cfg_find_cfg(svr_cfg, "connect-to"));
    while((depend_cfg = cfg_it_next(&depends_it))) {
        center_agent_svr_type_t depend_svr_type;
        const char * depend_svr_name = cfg_as_string(depend_cfg, NULL);

        if (depend_svr_name == NULL) {
            CPE_ERROR(conn_svr->m_em, "%s: create: depend format type error!", conn_svr_name(conn_svr));
            return -1;
        }

        depend_svr_type = center_agent_svr_type_check_create(conn_svr->m_agent, depend_svr_name);
        if (depend_svr_type == NULL) {
            CPE_ERROR(conn_svr->m_em, "%s: create: depend svr %s not exist!", conn_svr_name(conn_svr), depend_svr_name);
            return -1;
        }

        depend_cfg = cfg_find_cfg(cfg_find_cfg(gd_app_cfg(app), "svr_types"), depend_svr_name);
        if (depend_cfg == NULL) {
            CPE_ERROR(conn_svr->m_em, "%s: create: depend cfg of %s not configured!", conn_svr_name(conn_svr), depend_svr_name);
            return -1;
        }

        if (conn_svr_init_backend(conn_svr, depend_svr_type, depend_cfg) != 0) return -1;
    }

    return 0;
}

