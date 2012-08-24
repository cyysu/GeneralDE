#ifndef TSF4WG_TCAPLUS_INTERNAL_TYPES_H
#define TSF4WG_TCAPLUS_INTERNAL_TYPES_H
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

    struct mem_buffer m_dump_buffer;
    uint32_t m_dump_buffer_capacity;

    int m_debug;
};

struct mongo_request {
    mongo_driver_t m_driver;
    dp_req_t m_dp_req;
    int32_t m_finished;
    int32_t m_stack[32];
    int32_t m_stackPos;
    uint32_t m_reserve;
    struct mongo_pro_header m_pro_head;
};

#endif
