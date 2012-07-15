#ifndef USF_BPG_BIND_INTERNAL_TYPES_H
#define USF_BPG_BIND_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "cpe/dp/dp_types.h"
#include "usf/logic/logic_types.h"
#include "usf/bpg_rsp/bpg_rsp_types.h"
#include "usf/bpg_bind/bpg_bind_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bpg_bind_manage {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    bpg_rsp_manage_t m_rsp_mgr;

    dp_rsp_t m_recv_at;

    struct cpe_hash_table m_cliensts;
    struct cpe_hash_table m_connections;

    int m_debug;
};

struct bpg_bind_binding {
    uint64_t m_client_id;
    uint32_t m_connection_id;

    struct cpe_hash_entry m_hh_client;
    struct cpe_hash_entry m_hh_connection;
};

typedef enum bpg_net_pkg_next_step {
    bpg_net_pkg_next_go_with_connection_id
    , bpg_net_pkg_next_go_without_connection_id
    , bpg_net_pkg_next_ignore
    , bpg_net_pkg_next_close
} bpg_net_pkg_next_step_t;


#ifdef __cplusplus
}
#endif

#endif
