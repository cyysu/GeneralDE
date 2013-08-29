#ifndef SVR_SET_SVR_TYPES_CENTER_H
#define SVR_SET_SVR_TYPES_CENTER_H
#include "set_svr_types.h"

enum set_svr_center_state {
    set_svr_center_state_disable
    , set_svr_center_state_disconnected
    , set_svr_center_state_connecting
    , set_svr_center_state_join
    , set_svr_center_state_idle
    , set_svr_center_state_syncing
};

enum set_svr_center_fsm_evt_type {
    set_svr_center_fsm_evt_pkg
    , set_svr_center_fsm_evt_start
    , set_svr_center_fsm_evt_stop
    , set_svr_center_fsm_evt_timeout
    , set_svr_center_fsm_evt_connected
    , set_svr_center_fsm_evt_disconnected
    , set_svr_center_fsm_evt_wb_update
};

struct set_svr_center_fsm_evt {
    enum set_svr_center_fsm_evt_type m_type;
    SVR_CENTER_PKG * m_pkg;
};

struct set_svr_center {
    set_svr_t m_svr;

    fsm_def_machine_t m_fsm_def;
    struct fsm_machine m_fsm;
    gd_timer_id_t m_fsm_timer_id;

    uint32_t m_read_block_size;
    uint32_t m_max_pkg_size;
    uint32_t m_conn_id;

    uint32_t m_reconnect_span_ms;
    uint32_t m_update_span_s;
    char m_ip[16];
    uint16_t m_port;

    LPDRMETA m_pkg_meta;

    ringbuffer_block_t m_rb;
    ringbuffer_block_t m_wb;
    ringbuffer_block_t m_tb;

    int m_fd;
    struct ev_io m_watcher;

    struct mem_buffer m_outgoing_pkg_buf;
};

#endif
