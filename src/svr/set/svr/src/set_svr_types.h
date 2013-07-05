#ifndef SVR_SET_SVR_TYPES_H
#define SVR_SET_SVR_TYPES_H
#include "ev.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/utils/ringbuffer.h"
#include "cpe/net/net_types.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/timer/timer_manage.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "svr/set/share/set_share_types.h"
#include "protocol/svr/set/svr_set_internal.h"
#include "protocol/svr/center/svr_center_pro.h"

typedef struct set_svr_mon * set_svr_mon_t;
typedef struct set_svr_mon_app * set_svr_mon_app_t;
typedef struct set_svr * set_svr_t;
typedef struct set_svr_router * set_svr_router_t;
typedef struct set_svr_router_conn * set_svr_router_conn_t;
typedef struct set_svr_svr_type * set_svr_svr_type_t;
typedef struct set_svr_svr * set_svr_svr_t;
typedef struct set_svr_center * set_svr_center_t;

typedef TAILQ_HEAD(set_svr_svr_list, set_svr_svr) set_svr_svr_list_t;
typedef TAILQ_HEAD(set_svr_router_conn_list, set_svr_router_conn) set_svr_router_conn_list_t;

struct set_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    char m_repository_root[128];
    char m_set_type[64];
    uint16_t m_set_id;

    struct ev_loop * m_ev_loop;
    uint32_t m_max_conn_id;

    set_svr_center_t m_center;
    set_svr_mon_t m_mon;

    fsm_def_machine_t m_router_conn_fsm_def;

    char m_router_ip[16];
    uint16_t m_router_port;
    uint16_t m_router_process_count_per_tick;
    uint32_t m_router_conn_timeout_ms;
    uint32_t m_router_read_block_size;
    uint32_t m_router_reconnect_span_ms;
    uint32_t m_router_max_pkg_size;

    ringbuffer_t m_ringbuf;
    
    uint16_t m_local_svr_count;
    set_svr_svr_list_t m_local_svrs;
    struct cpe_hash_table m_svrs;

    struct cpe_hash_table m_svr_types_by_id;
    struct cpe_hash_table m_svr_types_by_name;

    struct cpe_hash_table m_routers_by_id;
    struct cpe_hash_table m_routers_by_addr;
    set_svr_router_conn_list_t m_accept_router_conns;

    dp_req_t m_incoming_buf;
    struct mem_buffer m_dump_buffer_head;
    struct mem_buffer m_dump_buffer_carry;
    struct mem_buffer m_dump_buffer_body;
};

struct set_svr_svr_type {
    set_svr_t m_svr;

    uint16_t m_svr_type_id;
    char * m_svr_type_name;

    LPDRMETA m_pkg_meta;

    set_svr_svr_list_t m_svrs;

    struct cpe_hash_entry m_hh_by_id;
    struct cpe_hash_entry m_hh_by_name;
};

enum set_svr_svr_category {
    set_svr_svr_local = 1
    , set_svr_svr_remote = 2
};

struct set_svr_svr {
    set_svr_t m_svr;
    set_svr_svr_type_t m_svr_type;
    enum set_svr_svr_category m_category;
    uint16_t m_svr_type_id;
    uint16_t m_svr_id;

    set_chanel_t m_chanel;
    set_svr_router_t m_router;

    TAILQ_ENTRY(set_svr_svr) m_next_for_type;
    TAILQ_ENTRY(set_svr_svr) m_next_for_local;
    TAILQ_ENTRY(set_svr_svr) m_next_for_router;

    struct cpe_hash_entry m_hh;
};

#endif
