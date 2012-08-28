#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_manage.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "mongo_agent_internal_ops.h"

EXPORT_DIRECTIVE
int mongo_agent_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    mongo_agent_t agent;
    logic_manage_t logic_manage;
    const char * outgoing_send_to;
    const char * incoming_recv_at;

    outgoing_send_to = cfg_get_string(cfg, "outgoing-send-to", NULL);
    if (outgoing_send_to == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: outgoing-send-to not configured!",
            gd_app_module_name(module));
        return -1;
    }

    incoming_recv_at = cfg_get_string(cfg, "incoming-recv-at", NULL);
    if (incoming_recv_at == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: incoming-recv-at not configured!",
            gd_app_module_name(module));
        return -1;
    }

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

    if (mongo_agent_set_outgoing_send_to(agent, outgoing_send_to) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set outgoing-sent-to %s fail!", 
            gd_app_module_name(module), outgoing_send_to);
        mongo_agent_free(agent);
        return -1;
    }

    if (mongo_agent_set_incoming_recv_at(agent, incoming_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set incoming-recv-at %s fail!", 
            gd_app_module_name(module), incoming_recv_at);
        mongo_agent_free(agent);
        return -1;
    }

    agent->m_debug = cfg_get_int32(cfg, "debug", agent->m_debug);

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
