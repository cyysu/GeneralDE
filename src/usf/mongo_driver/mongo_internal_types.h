#ifndef USF_MONGO_DRIVER_INTERNAL_TYPES_H
#define USF_MONGO_DRIVER_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/dp/dp_types.h"
#include "cpe/net/net_types.h"
#include "usf/mongo_driver/mongo_driver_types.h"
#include "usf/mongo_driver/mongo_protocol.h"

struct mongo_seed {
    mongo_driver_t m_driver;
    char m_host[255];
    int m_port;
    net_connector_t m_connector;
    TAILQ_ENTRY(mongo_seed) m_next;
};

typedef TAILQ_HEAD(mongo_seed_list, mongo_seed) mongo_seed_list_t;

enum mongo_server_state {
    mongo_server_state_init
    , mongo_server_state_connecting
    , mongo_server_state_checking_is_master
    , mongo_server_state_connected
    , mongo_server_state_error
};

struct mongo_server {
    mongo_driver_t m_driver;
    char m_host[255];
    int m_port;
    net_connector_t m_connector;
    enum mongo_server_state m_state;
    uint32_t m_max_bson_size;

    TAILQ_ENTRY(mongo_server) m_next;
};

typedef TAILQ_HEAD(mongo_server_list, mongo_server) mongo_server_list_t;

struct mongo_driver {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    mongo_driver_state_t m_state;

    int m_connecting_seed_count;
    int m_connecting_server_count;

    int m_seed_count;
    mongo_seed_list_t m_seeds;
    int m_server_count;
    mongo_server_list_t m_servers;
    struct mongo_server * m_master_server;
    size_t m_server_read_chanel_size;
    size_t m_server_write_chanel_size;

    int m_conn_timeout_ms;
    int m_op_timeout_ms;
    int m_max_bson_size;

    cpe_hash_string_t m_incoming_send_to;
    dp_rsp_t m_outgoing_recv_at;

    size_t m_pkg_buf_max_size;
    mongo_pkg_t m_pkg_buf;

    struct mem_buffer m_dump_buffer;
    uint32_t m_dump_buffer_capacity;

    int m_debug;
};

struct mongo_pro_header {
    int32_t id;
    int32_t response_to;
    int32_t op;
};

union mongo_pro_data {
    struct mongo_pro_data_update m_update;
    struct mongo_pro_data_insert m_insert;
    struct mongo_pro_data_query m_query;
    struct mongo_pro_data_get_more m_get_more;
    struct mongo_pro_data_delete m_delete;
    struct mongo_pro_data_kill_cursors m_kill_cursor;
    struct mongo_pro_data_reply m_reply;
};

struct mongo_pkg {
    mongo_driver_t m_driver;
    dp_req_t m_dp_req;
    int32_t m_stack[32];
    int32_t m_stackPos;
    uint32_t m_reserve;
    char m_db[32];
    char m_collection[32];
    int32_t m_doc_count;
    int32_t m_cur_doc_start;
    int32_t m_cur_doc_pos;
    struct mongo_pro_header m_pro_head;
    union mongo_pro_data m_pro_data;
};

enum mongo_pkg_recv_result {
    mongo_pkg_recv_error = -1
    , mongo_pkg_recv_ok = 0
    , mongo_pkg_recv_not_enough_data = 1
};

#define MONGO_EMPTY_DOCUMENT_SIZE (5)
#define MONGO_DEFAULT_MAX_BSON_SIZE 4 * 1024 * 1024

#endif
