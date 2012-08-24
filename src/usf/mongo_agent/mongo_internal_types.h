#ifndef TSF4WG_TCAPLUS_INTERNAL_TYPES_H
#define TSF4WG_TCAPLUS_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/dp/dp_types.h"
#include "cpe/net/net_types.h"
#include "usf/mongo_agent/mongo_types.h"
#include "mongo_protocol.h"

struct mongo_host_port {
    char m_host[255];
    int m_port;

    TAILQ_ENTRY(mongo_host_port) m_next;
};

typedef TAILQ_HEAD(mongo_host_port_list, mongo_host_port) mongo_host_port_list_t;

struct mongo_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    logic_manage_t m_logic_mgr;
    mongo_agent_state_t m_state;

    net_connector_t m_connector;

    mongo_host_port_list_t m_seeds;
    mongo_host_port_list_t m_servers;
    int m_conn_timeout_ms;
    int m_op_timeout_ms;
    int m_max_bson_size;

    uint32_t m_runing_require_capacity;
    uint32_t m_runing_require_count;
    uint32_t m_runing_require_op_count;
    uint32_t m_runing_require_check_span;
    logic_require_id_t * m_runing_requires;

    struct mem_buffer m_dump_buffer;
    uint32_t m_dump_buffer_capacity;

    int m_debug;
};

struct mongo_request {
    mongo_agent_t m_agent;
    dp_req_t m_dp_req;
    int32_t m_finished;
    int32_t m_stack[32];
    int32_t m_stackPos;
    uint32_t m_reserve;
    struct mongo_pro_header m_pro_head;
};

#endif
