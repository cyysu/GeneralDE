#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_require.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "mongo_agent_internal_ops.h"

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
    agent->m_outgoing_send_to = NULL;
    agent->m_incoming_recv_at = NULL;

    agent->m_require_queue = logic_require_queue_create(app, alloc, em, name, logic_mgr);
    if (agent->m_require_queue == NULL) {
        CPE_ERROR(em, "%s: create: create logic_require_queue fail!", name);
        nm_node_free(agent_node);
        return NULL;
    }

    nm_node_set_type(agent_node, &s_nm_node_type_mongo_agent);

    return agent;
} 

static void mongo_agent_clear(nm_node_t node) {
    mongo_agent_t agent;

    agent = (mongo_agent_t)nm_node_data(node);

    logic_require_queue_free(agent->m_require_queue);

    if (agent->m_outgoing_send_to) {
        mem_free(agent->m_alloc, agent->m_outgoing_send_to);
        agent->m_outgoing_send_to = NULL;
    }

    if (agent->m_incoming_recv_at) {
        dp_rsp_free(agent->m_incoming_recv_at);
        agent->m_incoming_recv_at = NULL;
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

int mongo_agent_set_outgoing_send_to(mongo_agent_t agent, const char * outgoing_send_to) {
    size_t name_len = cpe_hs_len_to_binary_len(strlen(outgoing_send_to));
    cpe_hash_string_t buf;

    buf = mem_alloc(agent->m_alloc, name_len);
    if (buf == NULL) return -1;

    cpe_hs_init(buf, name_len, outgoing_send_to);

    if (agent->m_outgoing_send_to) mem_free(agent->m_alloc, agent->m_outgoing_send_to);

    agent->m_outgoing_send_to = buf;

    return 0;
}

int mongo_agent_set_incoming_recv_at(mongo_agent_t agent, const char * incoming_recv_at) {
    char name_buf[128];

    snprintf(name_buf, sizeof(name_buf), "%s.incoming-recv-rsp", incoming_recv_at);

    if (agent->m_incoming_recv_at) dp_rsp_free(agent->m_incoming_recv_at);

    agent->m_incoming_recv_at = dp_rsp_create(gd_app_dp_mgr(agent->m_app), name_buf);
    if (agent->m_incoming_recv_at == NULL) return -1;

    dp_rsp_set_processor(agent->m_incoming_recv_at, mongo_agent_recv, agent);

    return 0;
}
