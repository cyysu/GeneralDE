#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_manage.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "mongo_internal_ops.h"

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

    agent->m_runing_require_check_span = 
        cfg_get_uint32(cfg, "runing-require-check-span", agent->m_runing_require_check_span);

    agent->m_debug = cfg_get_int32(cfg, "debug", agent->m_debug);

    /* if (cfg_get_int32(cfg, "auto-connect", 0)) { */
    /*     if (mongo_agent_connect(agent) != 0) { */
    /*         CPE_ERROR(gd_app_em(app), "%s create: connect error!", gd_app_module_name(module)); */
    /*         goto CREATE_ERROR; */
    /*     } */
    /* } */

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
