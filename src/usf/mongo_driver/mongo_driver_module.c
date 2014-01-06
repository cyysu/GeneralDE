#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/string_utils.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "mongo_internal_ops.h"

static int mongo_driver_app_init_load_seeds(gd_app_context_t app, mongo_driver_t driver, cfg_t cfg) {
    struct cfg_it cfg_it;
    cfg_t child;

    cfg_it_init(&cfg_it, cfg);

    while((child = cfg_it_next(&cfg_it))) {
        const char * host = cfg_get_string(child, "host", NULL);
        int32_t port = cfg_get_int32(child, "port", -1);
        if (host == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-seed: ip not configured!",
                mongo_driver_name(driver));
            return -1;
        }

        if (port == -1) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-seed: port not configured!",
                mongo_driver_name(driver));
            return -1;
        }

        if (mongo_driver_add_seed(driver, host, port) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-seed: add ip %s port %d fail!",
                mongo_driver_name(driver), host, port);
            return -1;
        }
    }

    return 0;
}

static int mongo_driver_app_init_load_server_from_arg(gd_app_context_t app, mongo_driver_t driver, const char * arg_name) {
    int arg_pos;
    const char * db_svr;
    char arg_name_buff[64];

    if (arg_name == NULL) return 0;

    if (arg_name[0] != '$') {
        CPE_ERROR(
            gd_app_em(app), "%s: create: load-server from arg %s: arg format error!",
            mongo_driver_name(driver), arg_name);
        return -1;
    }

    snprintf(arg_name_buff, sizeof(arg_name_buff), "--%s", arg_name + 1);

    arg_pos = 0;
    while((db_svr = gd_app_arg_find_ex(app, arg_name_buff, &arg_pos))) {
        char * sep;
        int port;
        int host_len;
        char host[64];
        ++arg_pos;

        sep = strchr(db_svr, ':');
        if (sep == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-server from arg %s: command line arg %s format error!",
                mongo_driver_name(driver), arg_name, db_svr);
            return -1;
        }

        host_len = sep - db_svr;
        if (host_len + 1 > sizeof(host)) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-server from arg %s: command line arg %s host too long!",
                mongo_driver_name(driver), arg_name, db_svr);
            return -1;
        }

        memcpy(host, db_svr, host_len);
        host[host_len] = 0;

        port = atoi(sep + 1);
        
        if (mongo_driver_add_server(driver, host, port) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-server from arg %s: add ip %s port %d fail!",
                mongo_driver_name(driver), arg_name, host, port);
            return -1;
        }
    }

    return 0;
}

static int mongo_driver_app_init_load_servers(gd_app_context_t app, mongo_driver_t driver, cfg_t cfg) {
    struct cfg_it cfg_it;
    cfg_t child;

    cfg_it_init(&cfg_it, cfg);
    while((child = cfg_it_next(&cfg_it))) {
        const char * host = cfg_get_string(child, "host", NULL);
        int32_t port = cfg_get_int32(child, "port", -1);
        if (host == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-server: ip not configured!",
                mongo_driver_name(driver));
            return -1;
        }

        if (port == -1) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-server: port not configured!",
                mongo_driver_name(driver));
            return -1;
        }

        if (mongo_driver_add_server(driver, host, port) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-server: add ip %s port %d fail!",
                mongo_driver_name(driver), host, port);
            return -1;
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
int mongo_driver_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    const char * incoming_send_to;
    const char * outgoing_recv_at;
    mongo_driver_t driver;
    const char * str_ringbuf_size;
    uint64_t ringbuf_size;

    if ((str_ringbuf_size = cfg_get_string(cfg, "ringbuf-size", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: ringbuf-size not configured!", gd_app_module_name(module));
        return -1;
    }

    if (str_ringbuf_size[0] == '$') {
        char arg_name_buf[64];
        snprintf(arg_name_buf, sizeof(arg_name_buf), "--%s", str_ringbuf_size + 1);
        str_ringbuf_size = gd_app_arg_find(app, arg_name_buf);
        if (str_ringbuf_size == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read ringbuf-size %s not configured in command line!",
                gd_app_module_name(module), arg_name_buf);
            return -1;
        }
    }

    if (cpe_str_parse_byte_size(&ringbuf_size, str_ringbuf_size) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: read ringbuf-size %s fail!",
            gd_app_module_name(module), str_ringbuf_size);
        return -1;
    }

    incoming_send_to = cfg_get_string(cfg, "incoming-send-to", NULL);
    if (incoming_send_to == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: incoming-send-to not configured!",
            gd_app_module_name(module));
        return -1;
    }

    outgoing_recv_at = cfg_get_string(cfg, "outgoing-recv-at", NULL);
    if (outgoing_recv_at == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: outgoing-recv-at not configured!",
            gd_app_module_name(module));
        return -1;
    }

    driver = mongo_driver_create(app, gd_app_module_name(module), gd_app_alloc(app), gd_app_em(app));
    if (driver == NULL) return -1;

    driver->m_pkg_buf_max_size = cfg_get_uint32(cfg, "pkg-buf-max-size", driver->m_pkg_buf_max_size);
    driver->m_reconnect_span_s = cfg_get_uint32(cfg, "reconnect-span-s", driver->m_reconnect_span_s);
    driver->m_op_timeout_ms = cfg_get_uint32(cfg, "op-timeout-ms", driver->m_op_timeout_ms);
    driver->m_read_block_size = cfg_get_uint32(cfg, "read-block-size", driver->m_read_block_size);

    driver->m_debug = cfg_get_int32(cfg, "debug", driver->m_debug);

    if (mongo_driver_set_ringbuf_size(driver, ringbuf_size) != 0
        || mongo_driver_app_init_load_server_from_arg(app, driver, cfg_get_string(cfg, "server-arg-name", NULL)) != 0
        || mongo_driver_app_init_load_seeds(app, driver, cfg_find_cfg(cfg, "seeds")) != 0
        || mongo_driver_app_init_load_servers(app, driver, cfg_find_cfg(cfg, "servers")) != 0
        || mongo_driver_set_incoming_send_to(driver, incoming_send_to) != 0
        || mongo_driver_set_outgoing_recv_at(driver, outgoing_recv_at) != 0
        )
    {
        mongo_driver_free(driver);
        return -1;
    }

    if (cfg_get_int32(cfg, "auto-enable", 0)) {
        if (mongo_driver_enable(driver) != 0) {
            CPE_ERROR(gd_app_em(app), "%s create: enable error!", gd_app_module_name(module));
            mongo_driver_free(driver);
            return -1;
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
void mongo_driver_app_fini(gd_app_context_t app, gd_app_module_t module) {
    mongo_driver_t driver;

    driver = mongo_driver_find_nc(app, gd_app_module_name(module));
    if (driver) {
        mongo_driver_free(driver);
    }
}
