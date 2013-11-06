#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "svr/dblog/agent/dblog_agent.h"
#include "dblog_agent_ops.h"

static void dblog_agent_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_dblog_agent = {
    "dblog_agent",
    dblog_agent_clear
};
dblog_agent_t
dblog_agent_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct dblog_agent * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct dblog_agent));
    if (svr_node == NULL) return NULL;

    svr = (dblog_agent_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_debug = 0;

    nm_node_set_type(svr_node, &s_nm_node_type_dblog_agent);

    return svr;
}

static void dblog_agent_clear(nm_node_t node) {
    dblog_agent_t svr;
    svr = (dblog_agent_t)nm_node_data(node);
}

void dblog_agent_free(dblog_agent_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_dblog_agent) return;
    nm_node_free(svr_node);
}

gd_app_context_t dblog_agent_app(dblog_agent_t svr) {
    return svr->m_app;
}

dblog_agent_t
dblog_agent_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dblog_agent) return NULL;
    return (dblog_agent_t)nm_node_data(node);
}

dblog_agent_t
dblog_agent_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dblog_agent) return NULL;
    return (dblog_agent_t)nm_node_data(node);
}

const char * dblog_agent_name(dblog_agent_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
dblog_agent_name_hs(dblog_agent_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t dblog_agent_cur_time(dblog_agent_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}
