#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_connector.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

extern char g_metalib_svr_center_pro[];
static void center_agent_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_center_agent = {
    "svr_center_agent",
    center_agent_clear
};

center_agent_t
center_agent_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct center_agent * mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct center_agent));
    if (mgr_node == NULL) return NULL;

    mgr = (center_agent_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_cvt = NULL;
    mgr->m_connector = NULL;
    mgr->m_read_chanel_size = 4 * 1024;
    mgr->m_write_chanel_size = 1024;
    mgr->m_process_count_per_tick = 10;
    mgr->m_max_pkg_size = 1024 * 1024 * 5;

    mgr->m_center_pkg_sn = 0;
    mgr->m_center_pkg_send_time = 0;
    mgr->m_center_state = center_agent_center_state_init;

    mgr->m_pkg_meta =
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_center_pro, "svr_center_pkg");
    if (mgr->m_pkg_meta == NULL) {
        CPE_ERROR(em, "%s: create find pkg meta fail!", name);
        nm_node_free(mgr_node);
        return NULL;
    }

    mem_buffer_init(&mgr->m_incoming_pkg_buf, mgr->m_alloc);
    mem_buffer_init(&mgr->m_outgoing_encode_buf, mgr->m_alloc);
    mem_buffer_init(&mgr->m_dump_buffer, mgr->m_alloc);

    nm_node_set_type(mgr_node, &s_nm_node_type_center_agent);

    return mgr;
}

static void center_agent_clear(nm_node_t node) {
    center_agent_t mgr;
    mgr = (center_agent_t)nm_node_data(node);

    mem_buffer_clear(&mgr->m_incoming_pkg_buf);
    mem_buffer_clear(&mgr->m_outgoing_encode_buf);
    mem_buffer_clear(&mgr->m_dump_buffer);

    if (mgr->m_cvt) {
        dr_cvt_free(mgr->m_cvt);
        mgr->m_cvt = NULL;
    }

    if (mgr->m_connector) {
        net_connector_free(mgr->m_connector);
        mgr->m_connector = NULL;
    }
}

gd_app_context_t center_agent_app(center_agent_t mgr) {
    return mgr->m_app;
}

void center_agent_free(center_agent_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_center_agent) return;
    nm_node_free(mgr_node);
}

center_agent_t
center_agent_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_center_agent) return NULL;
    return (center_agent_t)nm_node_data(node);
}

center_agent_t
center_agent_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_center_agent) return NULL;
    return (center_agent_t)nm_node_data(node);
}

const char * center_agent_name(center_agent_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
center_agent_name_hs(center_agent_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

int center_agent_set_cvt(center_agent_t agent, const char * cvt_name) {
    if (agent->m_cvt) dr_cvt_free(agent->m_cvt);

    agent->m_cvt = dr_cvt_create(agent->m_app, cvt_name);
    if (agent->m_cvt == NULL) {
        CPE_ERROR(agent->m_em, "%s: set cvt %s fail!", center_agent_name(agent), cvt_name);
        return -1;
    } 

    return 0;
}

int center_agent_set_svr(center_agent_t agent, const char * ip, short port) {
    if (agent->m_connector) {
        net_connector_free(agent->m_connector);
        agent->m_connector = NULL;
    }
    
    agent->m_connector = 
        net_connector_create_with_ep(gd_app_net_mgr(agent->m_app), center_agent_name(agent), ip, port);
    if (agent->m_connector == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: set svr to %s:%d: create connector fail!",
            center_agent_name(agent), ip, (int)port);
        return -1;
    }
 
    if (center_agent_center_ep_init(agent, net_connector_ep(agent->m_connector)) != 0) {
        net_connector_free(agent->m_connector);
        agent->m_connector = NULL;
        CPE_ERROR(
            agent->m_em, "%s: set svr to %s:%d: init ep fail!",
            center_agent_name(agent), ip, (int)port);
        return -1;
    }

    return 0;
}

int center_agent_set_reconnect_span_ms(center_agent_t agent, uint32_t span_ms) {
    if (agent->m_connector == NULL) return -1;
    net_connector_set_reconnect_span_ms(agent->m_connector, span_ms);
    return 0;
}
