#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_types.h"

EXPORT_DIRECTIVE
int center_agent_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    center_agent_t center_agent;
    const char * ip;
    short port;
    const char * cvt_name;

    ip = cfg_get_string(cfg, "ip", "");
    port = cfg_get_int16(cfg, "port", 0);
    cvt_name = cfg_get_string(cfg, "pkg-cvt", "pbuf-len");

    center_agent =
        center_agent_create(app, gd_app_module_name(module), gd_app_alloc(app), gd_app_em(app));
    if (center_agent == NULL) return -1;

    center_agent->m_process_count_per_tick = cfg_get_uint32(cfg, "process-count-per-tick", center_agent->m_process_count_per_tick);
    center_agent->m_read_chanel_size = cfg_get_uint32(cfg, "read-chanel-size", center_agent->m_read_chanel_size);
    center_agent->m_write_chanel_size = cfg_get_uint32(cfg, "write-chanel-size", center_agent->m_write_chanel_size);
    center_agent->m_debug = cfg_get_int8(cfg, "debug", center_agent->m_debug);
    center_agent_set_reconnect_span_ms(center_agent, cfg_get_uint32(cfg, "reconnect-span-ms", 30000));
    center_agent->m_max_pkg_size = cfg_get_uint32(cfg, "max-pkg-size", center_agent->m_max_pkg_size);

    if (center_agent_set_cvt(center_agent, cvt_name) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set cvt %s fail!",
            gd_app_module_name(module), cvt_name);
        center_agent_free(center_agent);
        return -1;
    }

    if (center_agent_set_svr(center_agent, ip, port) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set svr %s:%d fail!",
            gd_app_module_name(module), ip, (int)port);
        center_agent_free(center_agent);
        return -1;
    }

    if (center_agent->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done. ip=%s, port=%u", gd_app_module_name(module), ip, port);
    }

    if (cfg_get_int32(cfg, "auto-enable", 0)) {
        if (net_connector_enable(center_agent->m_connector) == 0) {
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
