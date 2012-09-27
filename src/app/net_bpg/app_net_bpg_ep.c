#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_responser.h"
#include "gd/app/app_context.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "app/net_proxy/app_net_ep.h"
#include "app/net_bpg/app_net_bpg_ep.h"
#include "app_net_bpg_internal_ops.h"

struct nm_node_type s_nm_node_type_app_net_bpg_ep;

app_net_bpg_ep_t
app_net_bpg_ep_create(
    gd_app_context_t app,
    bpg_pkg_manage_t pkg_manage,
    logic_manage_t logic_manage,
    const char * name,
    app_net_proxy_t app_net_proxy,
    uint16_t app_type,
    uint16_t app_id,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    app_net_bpg_ep_t mgr;
    nm_node_t mgr_node;
    char name_buf[128];

    assert(app);
    assert(pkg_manage);
    assert(name);

    if (em == 0) em = gd_app_em(app);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct app_net_bpg_ep));
    if (mgr_node == NULL) return NULL;

    mgr = (app_net_bpg_ep_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_pkg_manage = pkg_manage;
    mgr->m_em = em;
    mgr->m_req_max_size = 4 * 1024;
    mgr->m_req_buf = NULL;
    mgr->m_debug = 0;
    mgr->m_dispatch_to = NULL;

    mem_buffer_init(&mgr->m_dump_buffer, alloc);
    mem_buffer_init(&mgr->m_rsp_buf, alloc);

    snprintf(name_buf, sizeof(name_buf), "%s.msg-queue", name);
    mgr->m_require_queue = logic_require_queue_create(app, alloc, em, name, logic_manage);
    if (mgr->m_require_queue == NULL) {
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_buffer_clear(&mgr->m_rsp_buf);
        nm_node_free(mgr_node);
        return NULL;
    }

    snprintf(name_buf, sizeof(name_buf), "%s.reply-rsp", name);
    mgr->m_reply_rsp = dp_rsp_create(gd_app_dp_mgr(app), name_buf);
    if (mgr->m_reply_rsp == NULL) {
        logic_require_queue_free(mgr->m_require_queue);
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_buffer_clear(&mgr->m_rsp_buf);
        nm_node_free(mgr_node);
        return NULL;
    }
    dp_rsp_set_processor(mgr->m_reply_rsp, app_net_bpg_ep_reply, mgr);

    mgr->m_ep = app_net_ep_create(app_net_proxy, app_type, app_id);
    if (mgr->m_ep == NULL) {
        logic_require_queue_free(mgr->m_require_queue);
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_buffer_clear(&mgr->m_rsp_buf);
        dp_rsp_free(mgr->m_reply_rsp);
        nm_node_free(mgr_node);
        return NULL;
    }


    nm_node_set_type(mgr_node, &s_nm_node_type_app_net_bpg_ep);

    return mgr;
}

static void app_net_bpg_ep_clear(nm_node_t node) {
    app_net_bpg_ep_t mgr;
    mgr = (app_net_bpg_ep_t)nm_node_data(node);

    mem_buffer_clear(&mgr->m_dump_buffer);
    mem_buffer_clear(&mgr->m_rsp_buf);

    dp_rsp_free(mgr->m_reply_rsp);
    mgr->m_reply_rsp = NULL;

    if (mgr->m_req_buf) {
        bpg_pkg_free(mgr->m_req_buf);
        mgr->m_req_buf = NULL;
    }

    if (mgr->m_dispatch_to) {
        mem_free(mgr->m_alloc, mgr->m_dispatch_to);
        mgr->m_dispatch_to = NULL;
    }

    logic_require_queue_free(mgr->m_require_queue);

    app_net_ep_free(mgr->m_ep);
}

void app_net_bpg_ep_free(app_net_bpg_ep_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_app_net_bpg_ep) return;
    nm_node_free(mgr_node);
}

app_net_bpg_ep_t
app_net_bpg_ep_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_app_net_bpg_ep) return NULL;
    return (app_net_bpg_ep_t)nm_node_data(node);
}

app_net_bpg_ep_t
app_net_bpg_ep_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_app_net_bpg_ep) return NULL;
    return (app_net_bpg_ep_t)nm_node_data(node);
}

gd_app_context_t app_net_bpg_ep_app(app_net_bpg_ep_t mgr) {
    return mgr->m_app;
}

const char * app_net_bpg_ep_name(app_net_bpg_ep_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
app_net_bpg_ep_name_hs(app_net_bpg_ep_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

int app_net_bpg_ep_set_dispatch_to(app_net_bpg_ep_t agent, const char * dispatch_to) {
    size_t name_len;

    if (agent->m_dispatch_to) {
        mem_free(agent->m_alloc, agent->m_dispatch_to);
        agent->m_dispatch_to = NULL;
    }

    if (dispatch_to) {
        name_len = cpe_hs_len_to_binary_len(strlen(dispatch_to));
        agent->m_dispatch_to = (cpe_hash_string_t)mem_alloc(agent->m_alloc, name_len);
        if (agent->m_dispatch_to == NULL) return -1;

        cpe_hs_init(agent->m_dispatch_to, name_len, dispatch_to);
    }

    return 0;
}

bpg_pkg_t
app_net_bpg_ep_req_buf(app_net_bpg_ep_t mgr) {
    if (mgr->m_req_buf) {
        if (bpg_pkg_pkg_data_capacity(mgr->m_req_buf) < mgr->m_req_max_size) {
            bpg_pkg_free(mgr->m_req_buf);
            mgr->m_req_buf = NULL;
        }
    }

    if (mgr->m_req_buf == NULL) {
        mgr->m_req_buf = bpg_pkg_create(mgr->m_pkg_manage, mgr->m_req_max_size, NULL, 0);
    }

    return mgr->m_req_buf;
}

static void app_net_bpg_ep_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_app_net_bpg_ep = {
    "usf_app_net_bpg_ep",
    app_net_bpg_ep_clear
};
