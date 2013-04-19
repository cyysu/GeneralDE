#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

static int center_agent_app_init_center(struct center_agent_center * center, cfg_t cfg) {
    const char * cvt_name;
    const char * ip;
    short port;

    cvt_name = cfg_get_string(cfg, "pkg-cvt", "pbuf-len");
    ip = cfg_get_string(cfg, "ip", "");
    port = cfg_get_int16(cfg, "port", 0);

    center->m_process_count_per_tick = cfg_get_uint32(cfg, "process-count-per-tick", center->m_process_count_per_tick);
    center->m_read_chanel_size = cfg_get_uint32(cfg, "read-chanel-size", center->m_read_chanel_size);
    center->m_write_chanel_size = cfg_get_uint32(cfg, "write-chanel-size", center->m_write_chanel_size);
    center_agent_center_set_reconnect_span_ms(center, cfg_get_uint32(cfg, "reconnect-span-ms", 30000));
    center->m_max_pkg_size = cfg_get_uint32(cfg, "max-pkg-size", center->m_max_pkg_size);

    if (center_agent_center_set_cvt(center, cvt_name) != 0) {
        CPE_ERROR(
            center->m_agent->m_em, "%s: create: set center cvt %s fail!",
            center_agent_name(center->m_agent), cvt_name);
        return -1;
    }

    if (center_agent_center_set_svr(center, ip, port) != 0) {
        CPE_ERROR(
            center->m_agent->m_em, "%s: create: set svr %s:%d fail!",
            center_agent_name(center->m_agent), ip, (int)port);
        return -1;
    }

    return 0;
}

EXPORT_DIRECTIVE
int center_agent_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    center_agent_t center_agent;
    uint16_t svr_type;
    uint16_t svr_id;
    uint16_t listen_port;
    struct cfg_it sync_svr_types;
    cfg_t sync_svr_type;

    if (cfg_try_get_uint16(cfg, "svr-type", &svr_type) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: svr-type not configured!",
            gd_app_module_name(module));
        return -1;
    }

    if (cfg_try_get_uint16(cfg, "svr-id", &svr_id) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: svr-id not configured!",
            gd_app_module_name(module));
        return -1;
    }

    if (cfg_try_get_uint16(cfg, "svr.port", &listen_port) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: svr.port not configured!",
            gd_app_module_name(module));
        return -1;
    }

    center_agent =
        center_agent_create(
            app, gd_app_module_name(module),
            svr_type, svr_id, listen_port,
            gd_app_alloc(app), gd_app_em(app));
    if (center_agent == NULL) return -1;

    center_agent->m_debug = cfg_get_int8(cfg, "debug", center_agent->m_debug);

    cfg_it_init(&sync_svr_types, cfg_find_cfg(cfg, "synch-svrs"));
    while((sync_svr_type = cfg_it_next(&sync_svr_types))) {
        uint16_t svr_type;
        if (cfg_try_as_uint16(sync_svr_type, &svr_type) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read synch svr type fail!",
                gd_app_module_name(module));
            center_agent_free(center_agent);
            return -1;
        }

        if (center_agent_data_group_create(center_agent, svr_type) == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create synch svr %d fail!",
                gd_app_module_name(module), svr_type);
            center_agent_free(center_agent);
            return -1;
        }
    }

    if (center_agent_app_init_center(&center_agent->m_center, cfg_find_cfg(cfg, "center")) != 0) {
        center_agent_free(center_agent);
        return -1;
    }

    if (center_agent->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done. svr-type=%d, svr-id=%d, listen=port=%d",
            gd_app_module_name(module), svr_type, svr_id, listen_port);
    }

    if (cfg_get_int32(cfg, "auto-enable", 0)) {
        if (net_connector_enable(center_agent->m_center.m_connector) == 0) {
            if (center_agent->m_debug) {
                CPE_INFO(gd_app_em(app), "%s: create: auto-enable success!", gd_app_module_name(module));
            }
        }
        else {
            CPE_ERROR(gd_app_em(app), "%s: create: auto-enable fail!", gd_app_module_name(module));
            center_agent_free(center_agent);
            return -1;
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
void center_agent_app_fini(gd_app_context_t app, gd_app_module_t module) {
    center_agent_t center_agent;

    center_agent = center_agent_find_nc(app, gd_app_module_name(module));
    if (center_agent) {
        center_agent_free(center_agent);
    }
}
