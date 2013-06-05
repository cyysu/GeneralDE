#ifndef SVR_CENTER_SVR_TYPES_H
#define SVR_CENTER_SVR_TYPES_H
#include "ev.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/ringbuffer.h"
#include "cpe/utils/error.h"
#include "gd/app/app_types.h"
#include "cpe/aom/aom_types.h"
#include "protocol/svr/center/svr_center_internal.h"

typedef struct center_svr * center_svr_t;
typedef struct center_svr_relation * center_svr_relation_t;
typedef struct center_svr_conn * center_svr_conn_t;
typedef struct center_cli_data * center_cli_data_t;
typedef struct center_cli_group * center_cli_group_t;
typedef struct center_cli_relation * center_cli_relation_t;

typedef TAILQ_HEAD(center_cli_relation_list, center_cli_relation) center_cli_relation_list_t;
typedef TAILQ_HEAD(center_cli_data_list, center_cli_data) center_cli_data_list_t;
 
struct center_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    LPDRMETA m_record_meta;
    LPDRMETA m_pkg_meta;

    struct ev_loop * m_ev_loop;

    uint32_t m_read_block_size;
    uint32_t m_process_count_per_tick;
    tl_time_span_t m_conn_timeout_ms;

    int m_fd;
    struct ev_io m_watcher;

    ringbuffer_t m_ringbuf;

    uint32_t m_max_pkg_size;

    struct cpe_hash_table m_conns;
    struct cpe_hash_table m_groups;

    struct mem_buffer m_mem_data_buf;
    aom_obj_mgr_t m_client_data_mgr;
    struct cpe_hash_table m_datas;

    struct mem_buffer m_outgoing_pkg_buf;
    struct mem_buffer m_dump_buffer;
};

/*组网服务组（就是服务类型）*/
struct center_cli_group {
    center_svr_t m_svr;
    char * m_svr_type_name;
    uint16_t m_svr_type;
    uint16_t m_svr_count;

    center_cli_relation_list_t m_providers; /*使用的服务类型*/
    center_cli_relation_list_t m_users;     /*使用我的服务类型*/
    center_cli_data_list_t m_datas;         /*服务实例列表*/

    struct cpe_hash_entry m_hh;
};

/*组网服务之间的关系*/
struct center_cli_relation {
    center_cli_group_t m_provider; /*服务提供端，server*/
    center_cli_group_t m_user;     /*服务使用端，client*/
    TAILQ_ENTRY(center_cli_relation) m_next_for_provider;
    TAILQ_ENTRY(center_cli_relation) m_next_for_user;
};    

struct center_svr_conn {
    center_svr_t m_svr;
    center_cli_data_t m_data;

    int m_fd;
    struct ev_io m_watcher;

    ringbuffer_block_t m_rb;
    ringbuffer_block_t m_wb;
    ringbuffer_block_t m_tb;

    struct cpe_hash_entry m_hh;
};

/*组网服务实例*/
struct center_cli_data {
    center_svr_t m_svr;
    center_cli_group_t m_group;       /*所属服务组*/
    center_svr_conn_t m_conn;         /*连接*/
    SVR_CENTER_CLI_RECORD * m_data;

    TAILQ_ENTRY(center_cli_data) m_next;

    struct cpe_hash_entry m_hh;
};

#endif
