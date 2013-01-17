#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/net/net_connector.h"
#include "gd/app/app_context.h"
#include "app/net_pkg/app_net_pkg.h"
#include "app/net_proxy/app_net_ep.h"
#include "app/net_proxy/app_net_proxy.h"
#include "app_net_proxy_internal_ops.h"

app_net_ep_t
app_net_ep_create(app_net_proxy_t proxy, uint16_t app_type, uint16_t app_id) {
    app_net_ep_t ep;

    ep = mem_alloc(proxy->m_alloc, sizeof(struct app_net_ep));
    if (ep == NULL) return NULL;

    ep->m_app_type = app_type;
    ep->m_app_id = app_id;
    ep->m_incoming_send_to = NULL;
    ep->m_outgoing_recv_at = NULL;
    ep->m_state = app_net_ep_state_init;

    cpe_hash_entry_init(&ep->m_hh);
    if (cpe_hash_table_insert_unique(&proxy->m_eps, ep) != 0) {
        CPE_ERROR(
            proxy->m_em, "%s: create app net ep %d: duplicate!",
            app_net_proxy_name(proxy), app_type);
        mem_free(proxy->m_alloc, ep);
        return NULL;
    }

    if (net_connector_state(proxy->m_connector) == net_connector_state_connected) {
        app_net_ep_send_regist_req(ep, net_connector_ep(proxy->m_connector));
    }

    return ep;
}

int app_net_ep_set_outgoing_recv_at(app_net_ep_t ep, const char * outgoing_recv_at) {
    if (ep->m_outgoing_recv_at) {
        dp_rsp_free(ep->m_outgoing_recv_at);
    }

    ep->m_outgoing_recv_at = dp_rsp_create(gd_app_dp_mgr(ep->m_proxy->m_app), outgoing_recv_at);
    if (ep->m_outgoing_recv_at == NULL) {
        return -1;
    }

    dp_rsp_set_processor(ep->m_outgoing_recv_at, app_net_ep_do_send, ep);

    return 0;
}

int app_net_ep_set_incoming_send_to(app_net_ep_t ep, const char * incoming_send_to) {
    if (ep->m_incoming_send_to) {
        mem_free(ep->m_proxy->m_alloc, ep->m_incoming_send_to);
    }

    ep->m_incoming_send_to = cpe_hs_create(ep->m_proxy->m_alloc, incoming_send_to);
    if (ep->m_incoming_send_to == NULL) {
        return -1;
    }

    return 0;
}

void app_net_ep_free(app_net_ep_t ep) {
    cpe_hash_table_remove_by_ins(&ep->m_proxy->m_eps, ep);

    if (ep->m_incoming_send_to) {
        mem_free(ep->m_proxy->m_alloc, ep->m_incoming_send_to);
        ep->m_incoming_send_to = NULL;
    }

    if (ep->m_outgoing_recv_at) {
        dp_rsp_free(ep->m_outgoing_recv_at);
        ep->m_outgoing_recv_at = NULL;
    }

    mem_free(ep->m_proxy->m_alloc, ep);
}

app_net_ep_t app_net_ep_find(app_net_proxy_t proxy, uint16_t app_type) {
    struct app_net_ep key;
    key.m_app_type = app_type;

    return cpe_hash_table_find(&proxy->m_eps, &key);
}

uint32_t app_net_ep_hash(const struct app_net_ep * ep) {
    return ((uint32_t)ep->m_app_type) << 12 | ep->m_app_id;
}

int app_net_ep_eq(const struct app_net_ep * l, const struct app_net_ep * r) {
    return l->m_app_type == r->m_app_type && l->m_app_id == r->m_app_id;
}

void app_net_ep_clear_all(app_net_proxy_t proxy) {
    struct cpe_hash_it ep_it;
    app_net_ep_t ep;

    cpe_hash_it_init(&ep_it, &proxy->m_eps);

    ep = cpe_hash_it_next(&ep_it);
    while (ep) {
        app_net_ep_t next = cpe_hash_it_next(&ep_it);
        app_net_ep_free(ep);
        ep = next;
    }
}

void app_net_ep_set_state(app_net_ep_t ep, app_net_ep_state_t state) {
    ep->m_state = state;
}

app_net_ep_state_t app_net_ep_state(app_net_ep_t ep) {
    return ep->m_state;
}

int app_net_ep_send(app_net_ep_t ep, app_net_pkg_t pkg) {
    return app_net_ep_do_send(app_net_pkg_to_dp_req(pkg), ep, ep->m_proxy->m_em);
}
