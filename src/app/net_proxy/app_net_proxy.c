#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/net/net_connector.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "app/net_pkg/app_net_pkg.h"
#include "app/net_proxy/app_net_proxy.h"
#include "app_net_proxy_internal_ops.h"

extern char g_metalib_app_net[];
struct nm_node_type s_nm_node_type_app_net_proxy;

app_net_proxy_t
app_net_proxy_create(
    gd_app_context_t app,
    app_net_pkg_manage_t pkg_manage,
    const char * name,
    const char * ip,
    short port,
    size_t read_chanel_size,
    size_t write_chanel_size,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    app_net_proxy_t mgr;
    nm_node_t mgr_node;

    assert(name);
    assert(ip);

    if (em == 0) em = gd_app_em(app);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct app_net_proxy));
    if (mgr_node == NULL) return NULL;

    mgr = (app_net_proxy_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_pkg_manage = pkg_manage;
    mgr->m_em = em;
    mgr->m_req_max_size = 4 * 1024;
    mgr->m_req_buf = NULL;
    mgr->m_debug = 0;

    mgr->m_cvt = NULL;
    mgr->m_head_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_app_net, "tspkg_head");
    if (mgr->m_head_meta == NULL) {
        nm_node_free(mgr_node);
        return NULL;
    }

    mem_buffer_init(&mgr->m_send_encode_buf, alloc);

    if (cpe_hash_table_init(
            &mgr->m_eps,
            mgr->m_alloc,
            (cpe_hash_fun_t) app_net_ep_hash,
            (cpe_hash_cmp_t) app_net_ep_eq,
            CPE_HASH_OBJ2ENTRY(app_net_ep, m_hh),
            -1) != 0)
    {
        mem_buffer_clear(&mgr->m_send_encode_buf);
        nm_node_free(mgr_node);
        return NULL;
    }

    mgr->m_connector =
        net_connector_create_with_ep(
            gd_app_net_mgr(app),
            name,
            ip,
            port);
    if (mgr->m_connector == NULL) {
        cpe_hash_table_fini(&mgr->m_eps);
        mem_buffer_clear(&mgr->m_send_encode_buf);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (app_net_proxy_ep_init(mgr, net_connector_ep(mgr->m_connector), read_chanel_size, write_chanel_size) != 0) {
        net_connector_free(mgr->m_connector);
        cpe_hash_table_fini(&mgr->m_eps);
        mem_buffer_clear(&mgr->m_send_encode_buf);
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_app_net_proxy);

    return mgr;
}

static void app_net_proxy_clear(nm_node_t node) {
    app_net_proxy_t mgr;
    mgr = (app_net_proxy_t)nm_node_data(node);

    if (mgr->m_req_buf) {
        app_net_pkg_free(mgr->m_req_buf);
        mgr->m_req_buf = NULL;
    }

    mem_buffer_clear(&mgr->m_send_encode_buf);

    net_connector_free(mgr->m_connector);

    app_net_ep_clear_all(mgr);
    cpe_hash_table_fini(&mgr->m_eps);
}

void app_net_proxy_free(app_net_proxy_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_app_net_proxy) return;
    nm_node_free(mgr_node);
}

app_net_proxy_t
app_net_proxy_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_app_net_proxy) return NULL;
    return (app_net_proxy_t)nm_node_data(node);
}

app_net_proxy_t
app_net_proxy_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_app_net_proxy) return NULL;
    return (app_net_proxy_t)nm_node_data(node);
}

gd_app_context_t app_net_proxy_app(app_net_proxy_t mgr) {
    return mgr->m_app;
}

const char * app_net_proxy_name(app_net_proxy_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
app_net_proxy_name_hs(app_net_proxy_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

int app_net_proxy_set_cvt(app_net_proxy_t mgr, const char * cvt) {
    dr_cvt_t new_cvt;

    if (mgr->m_cvt && strcmp(dr_cvt_name(mgr->m_cvt), cvt) == 0) return 0;

    new_cvt = dr_cvt_create(mgr->m_app, cvt);
    if (new_cvt == NULL) return -1;

    if (mgr->m_cvt) dr_cvt_free(mgr->m_cvt);

    mgr->m_cvt = new_cvt;
    return 0;
}

dr_cvt_t app_net_proxy_cvt(app_net_proxy_t mgr) {
    return mgr->m_cvt;
}

const char * app_net_proxy_cvt_name(app_net_proxy_t mgr) {
    return mgr->m_cvt ? dr_cvt_name(mgr->m_cvt) : "";
}

app_net_pkg_t
app_net_proxy_req_buf(app_net_proxy_t mgr) {
    if (mgr->m_req_buf) {
        if (app_net_pkg_data_capacity(mgr->m_req_buf) < mgr->m_req_max_size) {
            app_net_pkg_free(mgr->m_req_buf);
            mgr->m_req_buf = NULL;
        }
    }

    if (mgr->m_req_buf == NULL) {
        mgr->m_req_buf = app_net_pkg_create(mgr->m_pkg_manage, mgr->m_req_max_size);
    }

    return mgr->m_req_buf;
}

net_connector_t app_net_proxy_connector(app_net_proxy_t mgr) {
    return mgr->m_connector;
}

app_net_pkg_manage_t app_net_proxy_pkg_manage(app_net_proxy_t req) {
    return req->m_pkg_manage;
}

static void app_net_proxy_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_app_net_proxy = {
    "app_net_proxy",
    app_net_proxy_clear
};
