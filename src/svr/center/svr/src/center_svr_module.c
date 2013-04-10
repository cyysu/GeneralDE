#include <assert.h>
#include "cpe/pal/pal_external.h"
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
    int accept_queue_size;
    const char * cvt_name;
 
    ip = cfg_get_string(cfg, "ip", "");
    port = cfg_get_int16(cfg, "port", 0);
    accept_queue_size = cfg_get_int32(cfg, "accept-queue-size", 256);
    cvt_name = cfg_get_string(cfg, "pkg-cvt", "pbuf-len");

    center_svr =
        center_svr_create(
            app, gd_app_module_name(module),
            gd_app_alloc(app), gd_app_em(app));
    if (center_svr == NULL) return -1;


    center_svr->m_process_count_per_tick = cfg_get_uint32(cfg, "process-count-per-tick", center_svr->m_process_count_per_tick);
    center_svr->m_max_pkg_size = cfg_get_uint32(cfg, "max-pkg-size", center_svr->m_max_pkg_size);
    center_svr->m_read_chanel_size = cfg_get_uint32(cfg, "read-chanel-size", center_svr->m_read_chanel_size);
    center_svr->m_write_chanel_size = cfg_get_uint32(cfg, "write-chanel-size", center_svr->m_write_chanel_size);
    center_svr->m_conn_timeout_ms = cfg_get_uint32(cfg, "conn-timeout-ms", center_svr->m_conn_timeout_ms);

    center_svr->m_debug = cfg_get_int8(cfg, "debug", center_svr->m_debug);

    if (center_svr_set_cvt(center_svr, cvt_name) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set cvt %s fail!",
            gd_app_module_name(module), cvt_name);
        center_svr_free(center_svr);
        return -1;
    }

    if (center_svr_set_listener(center_svr, ip, port, accept_queue_size) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set listener %s:%d accept-queue-size=%d fail!",
            gd_app_module_name(module), ip, port, accept_queue_size);
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
        uint32_t capacity = cfg_get_uint32(cfg, "record-buf-size-m", 0) * 1024 * 1024;
        if (capacity == 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load from mem, mem-record-buf-size not configured or zero!",
                gd_app_module_name(module));
            center_svr_free(center_svr);
            return -1;
        }
        else if (center_svr_init_clients_from_mem(center_svr, capacity) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load from mem fail, size=%d!",
                gd_app_module_name(module), (int)capacity);
            center_svr_free(center_svr);
            return -1;
        }
    }

    if (center_svr->m_debug) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: done. ip=%s, port=%u, accept-queue-size=%d, timeout=%d(ms)",
            gd_app_module_name(module), ip, port, accept_queue_size, (int)center_svr->m_conn_timeout_ms);
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
