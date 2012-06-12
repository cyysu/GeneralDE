#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "usf/bpg_cli/bpg_cli_proxy.h"
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
    proxy->m_send_pkg_buf_size = 4 * 1024;
    proxy->m_send_pkg_buf = NULL;
    mem_buffer_init(&proxy->m_send_data_buf, gd_app_alloc(app));

    proxy->m_recv_at = NULL;
    /* sp->m_dsp = bpg_pkg_dsp_create(gd_app_alloc(app)); */
    /* if (sp->m_dsp == NULL) { */
    /*     CPE_ERROR(em, "%s: bpg_cli_sp_create: create dsp fail!", bpg_cli_sp_name(sp)); */
    /*     mem_buffer_clear(&sp->m_data_buf); */
    /*     mem_free(sp->m_alloc, sp); */
    /*     return NULL; */
    /* } */

    /* if (bpg_pkg_dsp_load(sp->m_dsp, cfg_find_cfg(cfg, "send-to"), em) != 0) { */
    /*     CPE_ERROR(em, "%s: bpg_cli_sp_create: load dsp fail!", bpg_cli_sp_name(sp)); */
    /*     bpg_pkg_dsp_free(sp->m_dsp); */
    /*     mem_buffer_clear(&sp->m_data_buf); */
    /*     mem_free(sp->m_alloc, sp); */
    /*     return NULL; */
    /* } */
    

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
        mem_free(proxy->m_alloc, proxy->m_send_to);
        proxy->m_send_to = NULL;
    }

    if (proxy->m_recv_at != NULL) {
        dp_rsp_free(proxy->m_recv_at);
        proxy->m_recv_at = NULL;
    }

    mem_buffer_clear(&proxy->m_send_data_buf);
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

int bpg_cli_proxy_send(
    bpg_cli_proxy_t proxy,
    logic_stack_node_t stack,
    bpg_pkg_t pkg)
{
    return 0;
}
