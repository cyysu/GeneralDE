#ifndef SVR_SET_SVR_ROUTER_TYPES_H
#define SVR_SET_SVR_ROUTER_TYPES_H
#include "set_svr_types.h"

enum set_svr_router_conn_state {
    set_svr_router_conn_state_connecting
    , set_svr_router_conn_state_established
    , set_svr_router_conn_state_registing
    , set_svr_router_conn_state_accepting
};

enum set_svr_router_conn_fsm_evt_type {
    set_svr_router_conn_fsm_evt_pkg
    , set_svr_router_conn_fsm_evt_timeout
    , set_svr_router_conn_fsm_evt_connected
    , set_svr_router_conn_fsm_evt_disconnected
    , set_svr_router_conn_fsm_evt_accepted
    , set_svr_router_conn_fsm_evt_registed
    , set_svr_router_conn_fsm_evt_wb_update
};

struct set_svr_router_conn_fsm_evt {
    enum set_svr_router_conn_fsm_evt_type m_type;
    void * m_pkg;
};

struct set_svr_router {
    set_svr_t m_svr;

    uint32_t m_id;

    uint32_t m_ip;
    uint16_t m_port;

    ringbuffer_block_t m_wb;

    set_svr_router_conn_t m_conn;
    set_svr_svr_list_t m_svr_svrs;

    struct cpe_hash_entry m_hh_by_addr;
    struct cpe_hash_entry m_hh_by_id;
};

struct set_svr_router_conn {
    set_svr_t m_svr;
    set_svr_router_t m_router;

    struct fsm_machine m_fsm;
    gd_timer_id_t m_fsm_timer_id;

    ringbuffer_block_t m_wb;
    ringbuffer_block_t m_rb;
    ringbuffer_block_t m_tb;

    int m_fd;
    struct ev_io m_watcher;

    TAILQ_ENTRY(set_svr_router_conn) m_next;
};

#endif
