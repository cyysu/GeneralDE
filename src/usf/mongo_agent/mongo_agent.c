#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "mongo_internal_ops.h"

static void mongo_agent_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_mongo_agent = {
    "usf_mongo_agent",
    mongo_agent_clear
};

mongo_agent_t
mongo_agent_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    mongo_agent_t agent;
    nm_node_t agent_node;

    agent_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct mongo_agent));
    if (agent_node == NULL) return NULL;

    agent = (mongo_agent_t)nm_node_data(agent_node);
    bzero(agent, sizeof(struct mongo_agent));

    agent->m_app = app;
    agent->m_alloc = alloc;
    agent->m_em = em;
    agent->m_debug = 0;
    agent->m_logic_mgr = logic_mgr;
    agent->m_runing_require_capacity = 0;
    agent->m_runing_require_count = 0;
    agent->m_runing_require_op_count = 0;
    agent->m_runing_require_check_span = 20000;
    agent->m_runing_requires = NULL;

    agent->m_dump_buffer_capacity = 4 * 1024;

    if (gd_app_tick_add(app, mongo_agent_tick, agent, (ptr_int_t)500) != 0) {
        nm_node_free(agent_node);
        return NULL;
    }

    mem_buffer_init(&agent->m_dump_buffer, agent->m_alloc);

    nm_node_set_type(agent_node, &s_nm_node_type_mongo_agent);

    return agent;
} 

static void mongo_agent_clear(nm_node_t node) {
    mongo_agent_t agent;

    agent = (mongo_agent_t)nm_node_data(node);

    mem_buffer_clear(&agent->m_dump_buffer);

    if (agent->m_runing_requires) {
        mem_free(agent->m_alloc, agent->m_runing_requires);
        agent->m_runing_requires = NULL;
    }
}

void mongo_agent_free(mongo_agent_t agent) {
    nm_node_t agent_node;
    assert(agent);

    agent_node = nm_node_from_data(agent);
    if (nm_node_type(agent_node) != &s_nm_node_type_mongo_agent) return;
    nm_node_free(agent_node);
}

mongo_agent_t
mongo_agent_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_agent) return NULL;
    return (mongo_agent_t)nm_node_data(node);
}

mongo_agent_t
mongo_agent_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_agent) return NULL;
    return (mongo_agent_t)nm_node_data(node);
}

gd_app_context_t mongo_agent_app(mongo_agent_t agent) {
    return agent->m_app;
}

const char * mongo_agent_name(mongo_agent_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
mongo_agent_name_hs(mongo_agent_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}
