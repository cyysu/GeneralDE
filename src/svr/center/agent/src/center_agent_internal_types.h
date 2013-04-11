#ifndef SVR_CENTER_AGENT_INTERNAL_TYPES_H
#define SVR_CENTER_AGENT_INTERNAL_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/net/net_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "svr/center/center_agent_types.h"

typedef enum center_agent_center_state {
    center_agent_center_state_init
    , center_agent_center_state_registing
    , center_agent_center_state_idle
} center_agent_center_state_t;


struct center_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    uint32_t m_process_count_per_tick;

    uint32_t m_center_pkg_sn;
    uint32_t m_center_pkg_send_time;
    center_agent_center_state_t m_center_state;

    LPDRMETA m_pkg_meta;
    dr_cvt_t m_cvt;

    uint32_t m_max_pkg_size;
    uint32_t m_read_chanel_size;
    uint32_t m_write_chanel_size;
    net_connector_t m_connector;

    struct mem_buffer m_incoming_pkg_buf;
    struct mem_buffer m_outgoing_encode_buf;
    struct mem_buffer m_dump_buffer;
};

#endif
