#ifndef APP_NET_PROXY_INTERNAL_TYPES_H
#define APP_NET_PROXY_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "cpe/net/net_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "app/net_proxy/app_net_proxy_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_net_proxy {
    gd_app_context_t m_app;
    app_net_pkg_manage_t m_pkg_manage;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    net_connector_t m_connector;

    LPDRMETA m_head_meta;
    dr_cvt_t m_cvt;

    size_t m_req_max_size;
    app_net_pkg_t m_req_buf;
    struct mem_buffer m_send_encode_buf;

    struct cpe_hash_table m_eps;

    int8_t m_debug;
};

struct app_net_ep {
    app_net_proxy_t m_proxy;
    uint16_t m_app_type;
    uint16_t m_app_id;
    app_net_ep_state_t m_state;
    dp_rsp_t m_outgoing_recv_at;
    cpe_hash_string_t m_incoming_send_to;
    struct cpe_hash_entry m_hh;
};

#ifdef __cplusplus
}
#endif

#endif
