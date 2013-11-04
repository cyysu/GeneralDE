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
#include "dblog_svr_ops.h"

static void dblog_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_dblog_svr = {
    "dblog_svr",
    dblog_svr_clear
};

dblog_svr_t
dblog_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct dblog_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct dblog_svr));
    if (svr_node == NULL) return NULL;

    svr = (dblog_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_debug = 0;
    svr->m_ev_loop = net_mgr_ev_loop(gd_app_net_mgr(app));
    svr->m_fd = -1;

    nm_node_set_type(svr_node, &s_nm_node_type_dblog_svr);

    return svr;
}

static void dblog_svr_clear(nm_node_t node) {
    dblog_svr_t svr;
    svr = (dblog_svr_t)nm_node_data(node);

    if (svr->m_fd >= 0) {
        ev_io_stop(svr->m_ev_loop, &svr->m_watcher);
        cpe_sock_close(svr->m_fd);
        svr->m_fd = -1;
    }
}

void dblog_svr_free(dblog_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_dblog_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t dblog_svr_app(dblog_svr_t svr) {
    return svr->m_app;
}

dblog_svr_t
dblog_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dblog_svr) return NULL;
    return (dblog_svr_t)nm_node_data(node);
}

dblog_svr_t
dblog_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dblog_svr) return NULL;
    return (dblog_svr_t)nm_node_data(node);
}

const char * dblog_svr_name(dblog_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
dblog_svr_name_hs(dblog_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t dblog_svr_cur_time(dblog_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}
