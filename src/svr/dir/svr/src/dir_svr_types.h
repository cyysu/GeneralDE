#ifndef SVR_DIR_SVR_TYPES_H
#define SVR_DIR_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/dir/svr_dir_pro.h"

typedef struct dir_svr * dir_svr_t;

typedef struct dir_svr_region * dir_svr_region_t;
typedef struct dir_svr_server * dir_svr_server_t;

typedef TAILQ_HEAD(dir_svr_region_list, dir_svr_region) dir_svr_region_list_t;
typedef TAILQ_HEAD(dir_svr_server_list, dir_svr_server) dir_svr_server_list_t;

struct dir_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    uint16_t m_game_id;
    int m_debug;

    cpe_hash_string_t m_send_to;
    dp_rsp_t m_recv_at;

    LPDRMETA m_meta_res_query_regions;
    LPDRMETA m_meta_res_query_servers;
    LPDRMETA m_meta_res_error;

    uint16_t m_region_count;
    dir_svr_region_list_t m_regions;
};

struct dir_svr_region {
    dir_svr_t m_svr;
    uint16_t m_region_id;
    char m_region_name[64];
    uint8_t m_region_state;
    dir_svr_server_list_t m_servers;

    TAILQ_ENTRY(dir_svr_region) m_next;
};

struct dir_svr_server {
    dir_svr_region_t m_region;
    char m_ip[16];
    uint16_t m_port;

    TAILQ_ENTRY(dir_svr_server) m_next;
};

#endif
