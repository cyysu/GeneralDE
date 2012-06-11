#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_require.h"
#include "usf/bpg_use/bpg_use_proxy.h"
#include "bpg_use_internal_types.h"

static void bpg_use_proxy_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_bpg_use_proxy = {
    "usf_bpg_use_proxy",
    bpg_use_proxy_clear
};

bpg_use_proxy_t
bpg_use_proxy_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    error_monitor_t em)
{
    bpg_use_proxy_t mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_use_proxy));
    if (mgr_node == NULL) return NULL;

    mgr = (bpg_use_proxy_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = gd_app_alloc(app);
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_logic_mgr = logic_mgr;

    nm_node_set_type(mgr_node, &s_nm_node_type_bpg_use_proxy);

    return mgr;
}

static void bpg_use_proxy_clear(nm_node_t node) {
    bpg_use_proxy_t mgr;
    mgr = (bpg_use_proxy_t)nm_node_data(node);

}

void bpg_use_proxy_free(bpg_use_proxy_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_bpg_use_proxy) return;
    nm_node_free(mgr_node);
}

bpg_use_proxy_t
bpg_use_proxy_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_use_proxy) return NULL;
    return (bpg_use_proxy_t)nm_node_data(node);
}

bpg_use_proxy_t
bpg_use_proxy_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_use_proxy) return NULL;
    return (bpg_use_proxy_t)nm_node_data(node);
}

int bpg_use_proxy_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    bpg_pkg_t pkg;
    uint32_t sn;
    logic_require_t require;
    struct bpg_use_proxy * proxy;

    proxy = (struct bpg_use_proxy *)ctx;

    pkg = bpg_pkg_from_dp_req(req);
    if (pkg == NULL) {
        CPE_ERROR(em, "bpg_use_proxy_rsp: cast to pkg fail!");
        return -1;
    }

    sn = bpg_pkg_sn(pkg);
    require = logic_require_find(proxy->m_logic_mgr, sn);
    if (require == NULL) {
        CPE_ERROR(em, "bpg_use_proxy_rsp: require not exist, sn=%u!", (unsigned int)sn);
        return -1;
    }

    
    return 0;
}
