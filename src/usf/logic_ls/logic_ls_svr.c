#include <assert.h>
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_manage.h"
#include "logic_ls_internal_ops.h"

static void logic_local_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_logic_local_svr = {
    "usf_logic_local_svr",
    logic_local_svr_clear
};

logic_local_svr_t
logic_local_svr_create(
    gd_app_context_t app,
    mem_allocrator_t alloc,
    const char * name,
    error_monitor_t em)
{
    struct logic_local_svr * mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct logic_local_svr));
    if (mgr_node == NULL) return NULL;

    mgr = (logic_local_svr_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;

    nm_node_set_type(mgr_node, &s_nm_node_type_logic_local_svr);

    return mgr;
}

static void logic_local_svr_clear(nm_node_t node) {
    logic_local_svr_t mgr;
    mgr = (logic_local_svr_t)nm_node_data(node);
}

gd_app_context_t logic_local_svr_app(logic_local_svr_t mgr) {
    return mgr->m_app;
}

void logic_local_svr_free(logic_local_svr_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_logic_local_svr) return;
    nm_node_free(mgr_node);
}

logic_local_svr_t
logic_local_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_logic_local_svr) return NULL;
    return (logic_local_svr_t)nm_node_data(node);
}

logic_local_svr_t
logic_local_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_logic_local_svr) return NULL;
    return (logic_local_svr_t)nm_node_data(node);
}

const char * logic_local_svr_name(logic_local_svr_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
logic_local_svr_name_hs(logic_local_svr_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}
