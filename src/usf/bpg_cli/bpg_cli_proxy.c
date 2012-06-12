#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "usf/bpg_cli/bpg_cli_proxy.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "bpg_cli_internal_types.h"
#include "protocol/bpg_cli_pkg_info.h"

static void bpg_cli_proxy_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_bpg_cli_proxy = {
    "usf_bpg_cli_proxy",
    bpg_cli_proxy_clear
};

bpg_cli_proxy_t
bpg_cli_proxy_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    bpg_pkg_manage_t pkg_manage,
    error_monitor_t em)
{
    bpg_cli_proxy_t proxy;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_cli_proxy));
    if (mgr_node == NULL) return NULL;

    proxy = (bpg_cli_proxy_t)nm_node_data(mgr_node);

    proxy->m_app = app;
    proxy->m_alloc = gd_app_alloc(app);
    proxy->m_em = em;
    proxy->m_debug = 0;
    proxy->m_logic_mgr = logic_mgr;
    proxy->m_pkg_manage = pkg_manage;

    proxy->m_send_to = NULL;
    proxy->m_send_pkg_max_size = 4 * 1024;
    proxy->m_send_pkg_buf = NULL;
    mem_buffer_init(&proxy->m_send_data_buf, gd_app_alloc(app));

    proxy->m_recv_at = NULL;

    nm_node_set_type(mgr_node, &s_nm_node_type_bpg_cli_proxy);

    return proxy;
}

static void bpg_cli_proxy_clear(nm_node_t node) {
    bpg_cli_proxy_t proxy;
    proxy = (bpg_cli_proxy_t)nm_node_data(node);

    if (proxy->m_send_pkg_buf) {
        bpg_pkg_free(proxy->m_send_pkg_buf);
        proxy->m_send_pkg_buf = NULL;
    }

    if (proxy->m_send_to != NULL) {
        bpg_pkg_dsp_free(proxy->m_send_to);
        proxy->m_send_to = NULL;
    }

    if (proxy->m_recv_at != NULL) {
        dp_rsp_free(proxy->m_recv_at);
        proxy->m_recv_at = NULL;
    }

    mem_buffer_clear(&proxy->m_send_data_buf);
}

gd_app_context_t bpg_cli_proxy_app(bpg_cli_proxy_t proxy) {
    return proxy->m_app;
}

void bpg_cli_proxy_free(bpg_cli_proxy_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_bpg_cli_proxy) return;
    nm_node_free(mgr_node);
}

bpg_cli_proxy_t
bpg_cli_proxy_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_cli_proxy) return NULL;
    return (bpg_cli_proxy_t)nm_node_data(node);
}

bpg_cli_proxy_t
bpg_cli_proxy_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_cli_proxy) return NULL;
    return (bpg_cli_proxy_t)nm_node_data(node);
}

const char * bpg_cli_proxy_name(bpg_cli_proxy_t proxy) {
    return nm_node_name(nm_node_from_data(proxy));
}

cpe_hash_string_t
bpg_cli_proxy_name_hs(bpg_cli_proxy_t proxy) {
    return nm_node_name_hs(nm_node_from_data(proxy));
}

size_t bpg_cli_proxy_buf_capacity(bpg_cli_proxy_t proxy) {
    return proxy->m_send_pkg_max_size;
}

void bpg_cli_proxy_set_buf_capacity(bpg_cli_proxy_t proxy, size_t capacity) {
    proxy->m_send_pkg_max_size = capacity;
}

int bpg_cli_proxy_set_send_to(bpg_cli_proxy_t proxy, cfg_t cfg) {
    if (proxy->m_send_to != NULL) {
        bpg_pkg_dsp_free(proxy->m_send_to);
        proxy->m_send_to = NULL;
    }

    proxy->m_send_to = bpg_pkg_dsp_create(proxy->m_alloc);
    if (proxy->m_send_to == NULL) return -1;

    if (bpg_pkg_dsp_load(proxy->m_send_to, cfg, proxy->m_em) != 0) return -1;

    return 0;
}

int bpg_cli_proxy_set_recv_at(bpg_cli_proxy_t proxy, const char * name) {
    char sp_name_buf[128];

    if (proxy->m_recv_at != NULL) {
        dp_rsp_free(proxy->m_recv_at);
        proxy->m_recv_at = NULL;
    }

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.recv.sp", bpg_cli_proxy_name(proxy));
    proxy->m_recv_at = dp_rsp_create(gd_app_dp_mgr(proxy->m_app), sp_name_buf);
    if (proxy->m_recv_at == NULL) {
        CPE_ERROR(
            proxy->m_em, "%s: bpg_cli_proxy_set_recv_at: create rsp fail!",
            bpg_cli_proxy_name(proxy));
        return -1;
    }

    if (dp_rsp_bind_string(proxy->m_recv_at, name, proxy->m_em) != 0) {
        CPE_ERROR(
            proxy->m_em, "%s: bpg_cli_proxy_set_recv_at: bind rsp to %s fail!",
            bpg_cli_proxy_name(proxy), name);
        dp_rsp_free(proxy->m_recv_at);
        proxy->m_recv_at = NULL;
        return -1;
    }

    return 0;
}

LPDRMETALIB bpg_cli_proxy_metalib(bpg_cli_proxy_t proxy) {
    return bpg_pkg_manage_data_metalib(proxy->m_pkg_manage);
}

LPDRMETA bpg_cli_proxy_meta(bpg_cli_proxy_t proxy, const char * name) {
    LPDRMETALIB metalib = bpg_pkg_manage_data_metalib(proxy->m_pkg_manage);
    return metalib == NULL
        ? NULL
        : dr_lib_find_meta_by_name(metalib, name);
}

void * bpg_cli_proxy_data_buf(bpg_cli_proxy_t proxy) {
    mem_buffer_set_size(&proxy->m_send_data_buf, proxy->m_send_pkg_max_size);
    return mem_buffer_make_continuous(&proxy->m_send_data_buf, 0);
}

bpg_pkg_manage_t bpg_cli_proxy_pkg_manage(bpg_cli_proxy_t proxy) {
    return proxy->m_pkg_manage;
}

bpg_pkg_t
bpg_cli_proxy_pkg_buf(bpg_cli_proxy_t proxy) {
    if (proxy->m_send_pkg_buf) {
        if (bpg_pkg_pkg_capacity(proxy->m_send_pkg_buf) < proxy->m_send_pkg_max_size) {
            bpg_pkg_free(proxy->m_send_pkg_buf);
            proxy->m_send_pkg_buf = NULL;
        }
    }

    if (proxy->m_send_pkg_buf == NULL) {
        proxy->m_send_pkg_buf = bpg_pkg_create(proxy->m_pkg_manage, proxy->m_send_pkg_max_size, NULL, 0);
    }

    return proxy->m_send_pkg_buf;
}

int bpg_cli_proxy_send(
    bpg_cli_proxy_t proxy,
    logic_require_t require,
    bpg_pkg_t pkg)
{
    int rv;

    if (proxy->m_send_to == NULL) {
        CPE_ERROR(
            proxy->m_em, "%s: bpg_cli_proxy_set_recv_at: no send to configured!",
            bpg_cli_proxy_name(proxy));
        logic_require_set_error(require);
        return -1;
    }

    bpg_pkg_set_sn(pkg, logic_require_id(require));

    rv = bpg_pkg_dsp_dispatch(proxy->m_send_to, pkg, proxy->m_em);

    if (rv != 0) {
        CPE_ERROR(
            proxy->m_em, "%s: bpg_cli_proxy_set_recv_at: dispatch to send fail!",
            bpg_cli_proxy_name(proxy));
        logic_require_set_error(require);
        return rv;
    }

    return 0;
}
