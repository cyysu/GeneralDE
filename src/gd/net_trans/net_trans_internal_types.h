#ifndef GD_NET_TRANS_INTERNAL_TYPES_H
#define GD_NET_TRANS_INTERNAL_TYPES_H
#include "curl/curl.h"
#include "ev.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/net_trans/net_trans_types.h"

typedef TAILQ_HEAD(net_trans_task_list, net_trans_task) net_trans_task_list_t;
 
struct net_trans_manage {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    struct ev_loop * m_loop;
	CURLM * m_multi_handle;

    int m_cfg_dns_cache_timeout;
    int m_cfg_connect_timeout_ms;
    int m_cfg_transfer_timeout_ms;
    int m_cfg_forbid_reuse;

    uint32_t m_max_id;
    struct cpe_hash_table m_groups;
    struct cpe_hash_table m_tasks;

    int m_debug;
};

struct net_trans_group {
    net_trans_manage_t m_mgr;
    const char * m_name;
    net_trans_task_list_t m_tasks;

    struct cpe_hash_entry m_hh_for_mgr;
};

struct net_trans_task {
    net_trans_group_t m_group;
    uint32_t m_id;
    size_t m_capacity;
    net_trans_task_state_t m_state;
    net_trans_task_result_t m_result;
    uint8_t m_in_callback;
    uint8_t m_is_free;
    CURL * m_handler;
    curl_socket_t m_sockfd;
    int m_evset;
    struct ev_io m_watch;
    net_trans_task_commit_op_t m_commit_op;
    void * m_commit_ctx;
    struct mem_buffer m_buffer;
    TAILQ_ENTRY(net_trans_task) m_next_for_group;
    struct cpe_hash_entry m_hh_for_mgr;
};

#endif

