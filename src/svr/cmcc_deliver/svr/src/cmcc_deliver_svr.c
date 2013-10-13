#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "cmcc_deliver_svr_ops.h"

extern char g_metalib_svr_cmcc_deliver_pro[];
static void cmcc_deliver_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_cmcc_deliver_svr = {
    "svr_cmcc_deliver_svr",
    cmcc_deliver_svr_clear
};

#define CMCC_DELIVER_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_cmcc_deliver_pro, __name); \
    assert(svr-> __arg)

cmcc_deliver_svr_t
cmcc_deliver_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct cmcc_deliver_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct cmcc_deliver_svr));
    if (svr_node == NULL) return NULL;

    svr = (cmcc_deliver_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_debug = 0;

    svr->m_send_to = NULL;
    svr->m_recv_at = NULL;

    CMCC_DELIVER_SVR_LOAD_META(m_pkg_meta_res_deliver, "svr_cmcc_deliver_res_deliver");

    nm_node_set_type(svr_node, &s_nm_node_type_cmcc_deliver_svr);

    return svr;
}

static void cmcc_deliver_svr_clear(nm_node_t node) {
    cmcc_deliver_svr_t svr;
    svr = (cmcc_deliver_svr_t)nm_node_data(node);
}

void cmcc_deliver_svr_free(cmcc_deliver_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_cmcc_deliver_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t cmcc_deliver_svr_app(cmcc_deliver_svr_t svr) {
    return svr->m_app;
}

cmcc_deliver_svr_t
cmcc_deliver_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_cmcc_deliver_svr) return NULL;
    return (cmcc_deliver_svr_t)nm_node_data(node);
}

cmcc_deliver_svr_t
cmcc_deliver_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_cmcc_deliver_svr) return NULL;
    return (cmcc_deliver_svr_t)nm_node_data(node);
}

const char * cmcc_deliver_svr_name(cmcc_deliver_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
cmcc_deliver_svr_name_hs(cmcc_deliver_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t cmcc_deliver_svr_cur_time(cmcc_deliver_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int cmcc_deliver_svr_set_send_to(cmcc_deliver_svr_t svr, const char * send_to) {
    cpe_hash_string_t new_send_to = cpe_hs_create(svr->m_alloc, send_to);
    if (new_send_to == NULL) return -1;

    if (svr->m_send_to) mem_free(svr->m_alloc, svr->m_send_to);
    svr->m_send_to = new_send_to;

    return 0;
}

int cmcc_deliver_svr_request_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int cmcc_deliver_svr_set_request_recv_at(cmcc_deliver_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_recv_at != NULL) dp_rsp_free(svr->m_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.cmcc_deliver.require", cmcc_deliver_svr_name(svr));
    svr->m_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: cmcc_deliver_svr_set_recv_at: create rsp fail!",
            cmcc_deliver_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_recv_at, cmcc_deliver_svr_request_rsp, svr);

    if (dp_rsp_bind_string(svr->m_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: cmcc_deliver_svr_set_recv_at: bind rsp to %s fail!",
            cmcc_deliver_svr_name(svr), name);
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
        return -1;
    }

    return 0;
}
