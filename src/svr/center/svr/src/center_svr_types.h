#ifndef SVR_CENTER_SVR_TYPES_H
#define SVR_CENTER_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/net/net_types.h"
#include "cpe/aom/aom_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "protocol/svr/center/svr_center_internal.h"

typedef struct center_svr * center_svr_t;
typedef struct center_cli_conn * center_cli_conn_t;
typedef struct center_cli_data * center_cli_data_t;
typedef struct center_cli_group * center_cli_group_t;

typedef TAILQ_HEAD(center_cli_conn_list, center_cli_conn) center_cli_conn_list_t;
typedef TAILQ_HEAD(center_cli_data_list, center_cli_data) center_cli_data_list_t;
 
struct center_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    LPDRMETA m_record_meta;
    LPDRMETA m_pkg_meta;
    dr_cvt_t m_cvt;

    uint32_t m_process_count_per_tick;

    uint32_t m_read_chanel_size;
    uint32_t m_write_chanel_size;
    tl_time_span_t m_conn_timeout_ms;
    net_listener_t m_listener;

    uint32_t m_max_pkg_size;
    struct mem_buffer m_incoming_pkg_buf;
    struct mem_buffer m_outgoing_pkg_buf;
    struct mem_buffer m_outgoing_encode_buf;

    center_cli_conn_list_t m_conns;

    struct cpe_hash_table m_groups;

    struct mem_buffer m_mem_data_buf;
    aom_obj_mgr_t m_client_data_mgr;
    struct cpe_hash_table m_datas;

    struct mem_buffer m_dump_buffer;
};

struct center_cli_group {
    center_svr_t m_svr;
    uint16_t m_svr_type;
    center_cli_data_list_t m_datas;

    struct cpe_hash_entry m_hh;
};

struct center_cli_conn {
    center_svr_t m_svr;
    net_ep_t m_ep;
    center_cli_data_t m_data;

    TAILQ_ENTRY(center_cli_conn) m_next;
};

struct center_cli_data {
    center_svr_t m_svr;
    center_cli_group_t m_group;
    center_cli_conn_t m_conn;
    SVR_CENTER_CLI_RECORD * m_data;

    TAILQ_ENTRY(center_cli_data) m_next;

    struct cpe_hash_entry m_hh;
};

#endif
