#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_manage.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "mongo_internal_ops.h"

static int mongo_agent_app_init_load_seeds(gd_app_context_t app, mongo_agent_t agent, cfg_t cfg) {
    struct cfg_it cfg_it;
    cfg_t child;

    cfg_it_init(&cfg_it, cfg);

    while((child = cfg_it_next(&cfg_it))) {
        const char * host = cfg_get_string(child, "host", NULL);
        int32_t port = cfg_get_int32(child, "port", -1);
        if (host == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-seed: ip not configured!",
                mongo_agent_name(agent));
            return -1;
        }

        if (port == -1) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-seed: port not configured!",
                mongo_agent_name(agent));
            return -1;
        }

        if (mongo_agent_add_seed(agent, host, port) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-seed: add ip %s port %d fail!",
                mongo_agent_name(agent), host, port);
            return -1;
        }
    }

    return 0;
}

static int mongo_agent_app_init_load_servers(gd_app_context_t app, mongo_agent_t agent, cfg_t cfg) {
    struct cfg_it cfg_it;
    cfg_t child;

    cfg_it_init(&cfg_it, cfg);

    while((child = cfg_it_next(&cfg_it))) {
        const char * host = cfg_get_string(child, "host", NULL);
        int32_t port = cfg_get_int32(child, "port", -1);
        if (host == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-server: ip not configured!",
                mongo_agent_name(agent));
            return -1;
        }

        if (port == -1) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-server: port not configured!",
                mongo_agent_name(agent));
            return -1;
        }

        if (mongo_agent_add_server(agent, host, port) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: load-server: add ip %s port %d fail!",
                mongo_agent_name(agent), host, port);
            return -1;
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
int mongo_agent_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    mongo_agent_t agent;
    logic_manage_t logic_manage;

    logic_manage = logic_manage_find_nc(app, cfg_get_string(cfg, "logic-manage", NULL));
    if (logic_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: logic-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "logic-manage", "default"));
        return -1;
    }

    agent = mongo_agent_create(app, gd_app_module_name(module), logic_manage, gd_app_alloc(app), gd_app_em(app));
    if (agent == NULL) return -1;

    if (mongo_agent_app_init_load_seeds(app, agent, cfg_find_cfg(cfg, "seeds")) != 0
        || mongo_agent_app_init_load_servers(app, agent, cfg_find_cfg(cfg, "servers")) != 0
        )
    {
        mongo_agent_free(agent);
        return -1;
    }

    agent->m_runing_require_check_span = 
        cfg_get_uint32(cfg, "runing-require-check-span", agent->m_runing_require_check_span);

    agent->m_debug = cfg_get_int32(cfg, "debug", agent->m_debug);

    if (cfg_get_int32(cfg, "auto-enable", 0)) {
        if (mongo_agent_enable(agent) != 0) {
            CPE_ERROR(gd_app_em(app), "%s create: enable error!", gd_app_module_name(module));
            mongo_agent_free(agent);
            return -1;
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
void mongo_agent_app_fini(gd_app_context_t app, gd_app_module_t module) {
    mongo_agent_t agent;

    agent = mongo_agent_find_nc(app, gd_app_module_name(module));
    if (agent) {
        mongo_agent_free(agent);
    }
}
