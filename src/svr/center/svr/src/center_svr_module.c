#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "center_svr_ops.h"

EXPORT_DIRECTIVE
int center_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    center_svr_t center_svr;
    const char * ip;
    short port;
    const char * str_ringbuf_size;
    uint64_t ringbuf_size;
    const char * str_read_block_size;
    const char * str_max_pkg_size;
 
    ip = cfg_get_string(cfg, "ip", "");
    port = cfg_get_int16(cfg, "port", 0);

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

    center_svr =
        center_svr_create(
            app, gd_app_module_name(module),
            gd_app_alloc(app), gd_app_em(app));
    if (center_svr == NULL) return -1;

    if (center_svr_set_ringbuf_size(center_svr, ringbuf_size) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set ringbuf-size %d fail!", gd_app_module_name(module), (int)ringbuf_size);
        center_svr_free(center_svr);
        return -1;
    }

    if ((str_read_block_size = cfg_get_string(cfg, "read-block-size", NULL))) {
        uint64_t read_block_size;
        if (cpe_str_parse_byte_size(&read_block_size, str_read_block_size) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read read-block-size %s fail!",
                gd_app_module_name(module), str_read_block_size);
            return -1;
        }

        center_svr->m_read_block_size = (uint32_t)read_block_size;
    }

    if ((str_max_pkg_size = cfg_get_string(cfg, "max-pkg-size", NULL))) {
        uint64_t max_pkg_size;
        if (cpe_str_parse_byte_size(&max_pkg_size, str_max_pkg_size) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read max-pkg-size %s fail!",
                gd_app_module_name(module), str_max_pkg_size);
            return -1;
        }

        center_svr->m_max_pkg_size = (uint32_t)max_pkg_size;
    }


    center_svr->m_process_count_per_tick = cfg_get_uint32(cfg, "process-count-per-tick", center_svr->m_process_count_per_tick);
    center_svr->m_conn_timeout_ms = cfg_get_uint32(cfg, "conn-timeout-ms", center_svr->m_conn_timeout_ms);

    center_svr->m_debug = cfg_get_int8(cfg, "debug", center_svr->m_debug);

    if (center_svr_load_svr_config(center_svr) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load svr config fail!", gd_app_module_name(module));
        center_svr_free(center_svr);
        return -1;
    }

    if (center_svr_start(center_svr, ip, port) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set listener %s:%d fail!",
            gd_app_module_name(module), ip, port);
        center_svr_free(center_svr);
        return -1;
    }

    if (cfg_get_uint32(cfg, "record-shm", 0)) {
        if (center_svr_init_clients_from_shm(center_svr, 0) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load from shm fail!",
                gd_app_module_name(module));
            center_svr_free(center_svr);
            return -1;
        }
    }
    else {
        const char * str_record_buf_size = cfg_get_string(cfg, "record-buf-size", NULL);
        uint64_t record_buf_size;
        if (str_record_buf_size == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load from mem, mem-record-buf-size not configured!",
                gd_app_module_name(module));
            center_svr_free(center_svr);
            return -1;
        }

        if (cpe_str_parse_byte_size(&record_buf_size, str_record_buf_size) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read record-buf-size %s fail!",
                gd_app_module_name(module), str_record_buf_size);
            center_svr_free(center_svr);
            return -1;
        }

        if (center_svr_init_clients_from_mem(center_svr, record_buf_size) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load from mem fail, size=%d!",
                gd_app_module_name(module), (int)record_buf_size);
            center_svr_free(center_svr);
            return -1;
        }
    }

    if (center_svr->m_debug) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: done. ip=%s, port=%u, timeout=%d(ms)",
            gd_app_module_name(module), ip, port, (int)center_svr->m_conn_timeout_ms);
    }

    CPE_INFO(
        gd_app_em(app),
        "%s: obj-size=%.2fk, buf-size=%.2fm, allocked=%d, free=%d\n",
        gd_app_module_name(module),
        ((float)sizeof(SVR_CENTER_CLI_RECORD)) / 1024.0,
        ((float)aom_obj_mgr_data_capacity(center_svr->m_client_data_mgr)) / 1024.0 / 1024.0,
        aom_obj_mgr_allocked_obj_count(center_svr->m_client_data_mgr),
        aom_obj_mgr_free_obj_count(center_svr->m_client_data_mgr));

    return 0;
}

EXPORT_DIRECTIVE
void center_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    center_svr_t center_svr;

    center_svr = center_svr_find_nc(app, gd_app_module_name(module));
    if (center_svr) {
        center_svr_free(center_svr);
    }
}
