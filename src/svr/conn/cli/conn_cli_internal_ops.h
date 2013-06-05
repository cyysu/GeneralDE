#ifndef SVR_CONN_CLI_INTERNAL_OPS_H
#define SVR_CONN_CLI_INTERNAL_OPS_H
#include "conn_cli_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void conn_cli_disconnect(conn_cli_t cli);
void conn_cli_apply_evt(conn_cli_t cli, enum conn_cli_fsm_evt_type type);
int conn_cli_start_state_timer(conn_cli_t cli, tl_time_span_t span);
void conn_cli_stop_state_timer(conn_cli_t cli);

int conn_cli_set_ringbuf_size(conn_cli_t conn, size_t capacity);
void conn_cli_link_node_r(conn_cli_t cli, ringbuffer_block_t blk);
void conn_cli_link_node_w(conn_cli_t conn, ringbuffer_block_t blk);

void conn_cli_rw_cb(EV_P_ ev_io *w, int revents);

conn_cli_pkg_t conn_cli_incoming_pkg(conn_cli_t cli);

/*fsm impl operations*/
int conn_cli_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em);
int conn_cli_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em);
int conn_cli_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em);
int conn_cli_fsm_create_established(fsm_def_machine_t fsm_def, error_monitor_t em);


/*conn_cli_svr_stub operations*/
conn_cli_svr_stub_t conn_cli_svr_stub_create(conn_cli_t cli, const char * svr_type_name, uint16_t svr_type_id);
void conn_cli_svr_stub_free(struct conn_cli_svr_stub * svr);
void conn_cli_svr_stub_free_all(conn_cli_t cli);

int conn_cli_svr_stub_outgoing_recv(dp_req_t req, void * ctx, error_monitor_t em);

uint32_t conn_cli_svr_stub_hash(conn_cli_svr_stub_t svr);
int conn_cli_svr_stub_eq(conn_cli_svr_stub_t l, conn_cli_svr_stub_t r);

#ifdef __cplusplus
}
#endif

#endif
