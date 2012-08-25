#ifndef USF_MONGO_DRIVER_INTERNAL_TYPES_H
#define USF_MONGO_DRIVER_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/dp/dp_types.h"
#include "cpe/net/net_types.h"
#include "usf/mongo_driver/mongo_driver_types.h"
#include "mongo_protocol.h"

struct mongo_host_port {
    char m_host[255];
    int m_port;

    TAILQ_ENTRY(mongo_host_port) m_next;
};

typedef TAILQ_HEAD(mongo_host_port_list, mongo_host_port) mongo_host_port_list_t;

struct mongo_source_info {
    mongo_driver_t m_driver;
    int32_t m_source;
    cpe_hash_string_t m_incoming_dsp_to;
    dp_rsp_t m_outgoing_rsp;

    struct cpe_hash_entry m_hh;
};

struct mongo_driver {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    mongo_driver_state_t m_state;

    net_connector_t m_connector;

    mongo_host_port_list_t m_seeds;
    mongo_host_port_list_t m_servers;
    int m_conn_timeout_ms;
    int m_op_timeout_ms;
    int m_max_bson_size;

    struct cpe_hash_table m_source_infos;

    size_t m_req_max_size;
    mongo_pkg_t m_req_buf;

    struct mem_buffer m_dump_buffer;
    uint32_t m_dump_buffer_capacity;

    int m_debug;
};

struct mongo_pkg {
    mongo_driver_t m_driver;
    dp_req_t m_dp_req;
    int32_t m_finished;
    int32_t m_stack[32];
    int32_t m_stackPos;
    uint32_t m_reserve;
    struct mongo_pro_header m_pro_head;
    struct mongo_pro_reply_fields m_pro_replay_fields;
};

#endif
