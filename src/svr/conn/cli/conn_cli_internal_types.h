#ifndef CONN_CLI_INTERNAL_TYPES_H
#define CONN_CLI_INTERNAL_TYPES_H
#include "ev.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/ringbuffer.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/timer/timer_manage.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "svr/conn/cli/conn_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct conn_cli {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int8_t m_debug;
    char m_ip[16];
    short m_port;

    uint32_t m_read_block_size;
    uint32_t m_reconnect_span_ms;
    size_t m_max_pkg_size;
    struct ev_loop * m_ev_loop;

    fsm_def_machine_t m_fsm_def;
    struct fsm_machine m_fsm;
    gd_timer_id_t m_fsm_timer_id;

    int m_fd;
    struct ev_io m_watcher;

    ringbuffer_t m_ringbuf;
    ringbuffer_block_t m_rb;
    ringbuffer_block_t m_wb;
    ringbuffer_block_t m_tb;

    conn_cli_pkg_t m_incoming_pkg;
    dp_req_t m_incoming_body;

    struct mem_buffer m_dump_buffer;

    struct cpe_hash_table m_svrs;
};

struct conn_cli_svr_stub {
    conn_cli_t m_cli;
    uint16_t m_svr_type_id;
    char * m_svr_type_name;

    LPDRMETA m_pkg_meta;
    dr_cvt_t m_cvt;

    cpe_hash_string_t m_response_dispatch_to;
    cpe_hash_string_t m_notify_dispatch_to;
    dp_rsp_t m_outgoing_recv_at;

    struct cpe_hash_entry m_hh;
};

enum conn_cli_fsm_evt_type {
    conn_cli_fsm_evt_start
    , conn_cli_fsm_evt_stop
    , conn_cli_fsm_evt_timeout
    , conn_cli_fsm_evt_connected
    , conn_cli_fsm_evt_disconnected
};

struct conn_cli_fsm_evt {
    enum conn_cli_fsm_evt_type m_type;
};

struct conn_cli_pkg {
    conn_cli_t m_cli;
    dp_req_t m_dp_req;
    uint16_t m_svr_type;
    int8_t m_result;
    int8_t m_flags;
    uint32_t m_sn;
};

#ifdef __cplusplus
}
#endif

#endif
