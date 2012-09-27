#ifndef USF_BPG_NET_INTERNAL_TYPES_H
#define USF_BPG_NET_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "usf/logic/logic_types.h"
#include "usf/logic_use/logic_use_types.h"
#include "app/net_bpg/app_net_bpg_types.h"
#include "app/net_proxy/app_net_proxy_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_net_bpg_ep {
    gd_app_context_t m_app;
    bpg_pkg_manage_t m_pkg_manage;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    app_net_ep_t m_ep;

    size_t m_req_max_size;
    bpg_pkg_t m_req_buf;
    struct mem_buffer m_rsp_buf;

    size_t m_read_chanel_size;
    size_t m_write_chanel_size;

    tl_time_span_t m_conn_timeout;

    cpe_hash_string_t m_dispatch_to;

    dp_rsp_t m_reply_rsp;

    struct mem_buffer m_dump_buffer;

    logic_require_queue_t m_require_queue;

    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
