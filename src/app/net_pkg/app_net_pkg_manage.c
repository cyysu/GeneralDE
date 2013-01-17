#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "app/net_pkg/app_net_pkg.h"
#include "app/net_pkg/app_net_pkg_manage.h"
#include "app_net_pkg_internal_ops.h"

static void app_net_pkg_manage_clear(nm_node_t node);

static cpe_hash_string_buf s_app_net_pkg_manage_default_name = CPE_HS_BUF_MAKE("app_net_pkg_manage");

struct nm_node_type s_nm_node_type_app_net_pkg_manage = {
    "app_net_pkg_manage",
    app_net_pkg_manage_clear
};

app_net_pkg_manage_t
app_net_pkg_manage_create(
    gd_app_context_t app,
    const char * name,
    error_monitor_t em)
{
    app_net_pkg_manage_t mgr;
    nm_node_t mgr_node;

    assert(app);

    if (name == 0) name = cpe_hs_data((cpe_hash_string_t)&s_app_net_pkg_manage_default_name);
    if (em == 0) em = gd_app_em(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct app_net_pkg_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (app_net_pkg_manage_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = gd_app_alloc(app);
    mgr->m_em = em;
    mgr->m_debug = 0;

    nm_node_set_type(mgr_node, &s_nm_node_type_app_net_pkg_manage);

    return mgr;
}

static void app_net_pkg_manage_clear(nm_node_t node) {
    app_net_pkg_manage_t mgr;
    mgr = (app_net_pkg_manage_t)nm_node_data(node);

}

void app_net_pkg_manage_free(app_net_pkg_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_app_net_pkg_manage) return;
    nm_node_free(mgr_node);
}

app_net_pkg_manage_t
app_net_pkg_manage_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) name = (cpe_hash_string_t)&s_app_net_pkg_manage_default_name;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_app_net_pkg_manage) return NULL;
    return (app_net_pkg_manage_t)nm_node_data(node);
}

app_net_pkg_manage_t
app_net_pkg_manage_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return app_net_pkg_manage_default(app);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_app_net_pkg_manage) return NULL;
    return (app_net_pkg_manage_t)nm_node_data(node);
}

app_net_pkg_manage_t
app_net_pkg_manage_default(gd_app_context_t app) {
    return app_net_pkg_manage_find(app, (cpe_hash_string_t)&s_app_net_pkg_manage_default_name);
}

gd_app_context_t app_net_pkg_manage_app(app_net_pkg_manage_t mgr) {
    return mgr->m_app;
}

const char * app_net_pkg_manage_name(app_net_pkg_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
app_net_pkg_manage_name_hs(app_net_pkg_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

