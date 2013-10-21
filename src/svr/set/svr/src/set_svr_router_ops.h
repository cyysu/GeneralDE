#ifndef SVR_SET_SVR_ROUTER_OPS_H
#define SVR_SET_SVR_ROUTER_OPS_H
#include "set_svr_router_types.h"
#include "set_svr_ops.h"

LPDRMETA set_svr_get_pkg_meta(set_svr_t svr, dp_req_t head, set_svr_svr_type_t to_svr_type, set_svr_svr_type_t from_svr_type);

/*operations of set_svr_router*/
set_svr_router_t set_svr_router_create(set_svr_t svr, uint32_t ip, uint16_t port);
void set_svr_router_free(set_svr_router_t svr_router);
void set_svr_router_free_all(set_svr_t svr);

set_svr_router_t set_svr_router_find_by_addr(set_svr_t svr, uint32_t ip, uint16_t port);
set_svr_router_t set_svr_router_find_by_id(set_svr_t svr, uint32_t id);

int set_svr_router_send(set_svr_router_t router, dp_req_t body, dp_req_t head, dp_req_t carry, size_t * write_size);
void set_svr_router_clear_data(set_svr_router_t router);
void set_svr_router_link_node_w(set_svr_router_t router, ringbuffer_block_t blk);

uint32_t set_svr_router_hash_by_addr(set_svr_router_t o);
int set_svr_router_eq_by_addr(set_svr_router_t l, set_svr_router_t r);

uint32_t set_svr_router_hash_by_id(set_svr_router_t o);
int set_svr_router_eq_by_id(set_svr_router_t l, set_svr_router_t r);

/*operations of set_svr_router_conn*/
set_svr_router_conn_t set_svr_router_conn_create(set_svr_t svr, set_svr_router_t router, int fd);
void set_svr_router_conn_free(set_svr_router_conn_t conn);
void set_svr_router_conn_free_all(set_svr_t svr);

fsm_def_machine_t
set_svr_router_conn_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em);

int set_svr_router_conn_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_router_conn_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_router_conn_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_router_conn_fsm_create_established(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_router_conn_fsm_create_accepting(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_router_conn_fsm_create_registing(fsm_def_machine_t fsm_def, error_monitor_t em);

void set_svr_router_conn_set_router(set_svr_router_conn_t conn, set_svr_router_t router);

void set_svr_router_conn_apply_evt(struct set_svr_router_conn * router_conn, enum set_svr_router_conn_fsm_evt_type type);
void set_svr_router_conn_apply_pkg(struct set_svr_router_conn * router_conn, SVR_CENTER_PKG * pkg);

int set_svr_router_conn_start_state_timer(struct set_svr_router_conn * router_conn, tl_time_span_t span);
void set_svr_router_conn_stop_state_timer(struct set_svr_router_conn * router_conn);

void set_svr_router_conn_link_node_r(set_svr_router_conn_t conn, ringbuffer_block_t blk);
void set_svr_router_conn_link_node_w(set_svr_router_conn_t conn, ringbuffer_block_t blk);
int set_svr_router_conn_read_from_net(set_svr_router_conn_t conn, size_t require_size);
int set_svr_router_conn_write_to_net(set_svr_router_conn_t conn);

int set_svr_router_conn_r_buf(set_svr_router_conn_t conn, size_t require_size, void * * buf);
void set_svr_router_conn_r_erase(set_svr_router_conn_t conn, size_t size);

#endif
