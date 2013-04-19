#ifndef SVR_CENTER_AGENT_INTERNAL_TYPES_H
#define SVR_CENTER_AGENT_INTERNAL_TYPES_H
#include "ev.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/net/net_types.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/timer/timer_manage.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "svr/center/center_agent_types.h"
#include "protocol/svr/center/svr_center_pro.h"

typedef struct center_agent_data_svr * center_agent_data_svr_t;
typedef struct center_agent_data_group * center_agent_data_group_t;

typedef TAILQ_HEAD(center_agent_data_svr_list, center_agent_data_svr) center_agent_data_svr_list_t;

struct center_agent_center {
    center_agent_t m_agent;

    fsm_def_machine_t m_fsm_def;
    struct fsm_machine m_fsm;
    gd_timer_id_t m_fsm_timer_id;

    uint32_t m_process_count_per_tick;

    uint32_t m_pkg_sn;

    LPDRMETA m_pkg_meta;
    dr_cvt_t m_cvt;

    uint32_t m_max_pkg_size;
    uint32_t m_read_chanel_size;
    uint32_t m_write_chanel_size;
    net_connector_t m_connector;

    struct mem_buffer m_incoming_pkg_buf;
    struct mem_buffer m_outgoing_encode_buf;
};

struct center_agent_svr {
    center_agent_t m_agent;
    uint16_t m_port;

    LPDRMETA m_pkg_meta;
    dr_cvt_t m_cvt;

    int m_fd;
    ev_io m_watcher;

    cpe_hash_string_t m_dispatch_to;

    size_t m_incoming_capacity;
    center_agent_pkg_t m_incoming_pkg_buf;
};

struct center_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    uint16_t m_svr_type;
    uint16_t m_svr_id;

    struct cpe_hash_table m_svrs;
    struct cpe_hash_table m_groups;

    struct center_agent_svr m_svr;
    struct center_agent_center m_center;

    struct mem_buffer m_dump_buffer;
};

struct center_agent_data_group {
    center_agent_t m_agent;
    uint16_t m_svr_type;
    center_agent_data_svr_list_t m_svrs;

    struct cpe_hash_entry m_hh;
};

struct center_agent_data_svr {
    center_agent_t m_agent;
    center_agent_data_group_t m_group;
    uint16_t m_svr_type;
    uint16_t m_svr_id;
    uint32_t m_ip;
    uint16_t m_port;

    TAILQ_ENTRY(center_agent_data_svr) m_next;
    struct cpe_hash_entry m_hh;
};

struct center_agent_pkg {
    center_agent_t m_agent;
    dp_req_t m_dp_req;
};

enum center_agent_fsm_evt_type {
    center_agent_fsm_evt_pkg
    , center_agent_fsm_evt_timeout
    , center_agent_fsm_evt_connected
    , center_agent_fsm_evt_disconnected
};

struct center_agent_fsm_evt {
    enum center_agent_fsm_evt_type m_type;
    SVR_CENTER_PKG * m_pkg;
};

#endif
