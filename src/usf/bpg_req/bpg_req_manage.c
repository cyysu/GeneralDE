#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/logic/logic_manage.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_req/bpg_req_manage.h"
#include "bpg_req_internal_ops.h"

static void bpg_req_manage_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_bpg_req_manage = {
    "usf_bpg_req_manage",
    bpg_req_manage_clear
};

bpg_req_manage_t
bpg_req_manage_create(
    logic_manage_t logic,
    const char * name,
    mem_allocrator_t alloc)
{
    gd_app_context_t app;
    bpg_req_manage_t mgr;
    nm_node_t mgr_node;

    assert(logic);
    assert(name);

    app = logic_manage_app(logic);
    assert(app);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_req_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (bpg_req_manage_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_logic = logic;
    mgr->m_debug = 0;

    nm_node_set_type(mgr_node, &s_nm_node_type_bpg_req_manage);

    return mgr;
}

static void bpg_req_manage_clear(nm_node_t node) {
    bpg_req_manage_t mgr;
    mgr = (bpg_req_manage_t)nm_node_data(node);
}

void bpg_req_manage_free(bpg_req_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_bpg_req_manage) return;
    nm_node_free(mgr_node);
}

bpg_req_manage_t
bpg_req_manage_find(
    gd_app_context_t app,
    cpe_hash_string_t name)
{
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_req_manage) return NULL;
    return (bpg_req_manage_t)nm_node_data(node);
}

bpg_req_manage_t
bpg_req_manage_find_nc(
    gd_app_context_t app,
    const char * name)
{
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_req_manage) return NULL;
    return (bpg_req_manage_t)nm_node_data(node);
}

gd_app_context_t bpg_req_manage_app(bpg_req_manage_t mgr) {
    return mgr->m_app;
}

const char * bpg_req_manage_name(bpg_req_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
bpg_req_manage_name_hs(bpg_req_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

EXPORT_DIRECTIVE
int bpg_req_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    bpg_req_manage_t bpg_req_manage;
    logic_manage_t logic;
    const char * logic_name;

    logic_name = cfg_get_string(cfg, "logic", NULL);
    logic = logic_manage_find_nc(app, logic_name);
    if (logic == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: logic %s not exist!",
            gd_app_module_name(module),
            logic_name ? logic_name : "default");
    }

    bpg_req_manage = bpg_req_manage_create(logic, gd_app_module_name(module), gd_app_alloc(app));
    if (bpg_req_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: fail!",
            bpg_req_manage_name(bpg_req_manage))
        return -1;
    }

    bpg_req_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (bpg_req_manage->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            bpg_req_manage_name(bpg_req_manage));
    }

    return 0;
}

EXPORT_DIRECTIVE
void bpg_req_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_req_manage_t bpg_req_manage;

    bpg_req_manage = bpg_req_manage_find_nc(app, gd_app_module_name(module));
    if (bpg_req_manage) {
        bpg_req_manage_free(bpg_req_manage);
    }
}


