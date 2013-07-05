#ifndef SVR_SET_SVR_CENTER_OPS_H
#define SVR_SET_SVR_CENTER_OPS_H
#include "set_svr_center_types.h"
#include "set_svr_ops.h"
#include "protocol/svr/center/svr_center_pro.h"

set_svr_center_t set_svr_center_create(set_svr_t agent);
void set_svr_center_free(set_svr_center_t center);

int set_svr_app_init_mon(set_svr_mon_t mon);

int set_svr_center_set_svr(struct set_svr_center * center, const char * ip, short port);
int set_svr_center_set_reconnect_span_ms(struct set_svr_center * center, uint32_t span_ms);

int set_svr_center_send(struct set_svr_center * center, SVR_CENTER_PKG * pkg, size_t pkg_size);
void set_svr_center_disconnect(struct set_svr_center * center);

int set_svr_center_start_state_timer(struct set_svr_center * center, tl_time_span_t span);
void set_svr_center_stop_state_timer(struct set_svr_center * center);

uint32_t set_svr_center_fsm_trans_common(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt);
int set_svr_center_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_center_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_center_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_center_fsm_create_join(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_center_fsm_create_idle(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_center_fsm_create_syncing(fsm_def_machine_t fsm_def, error_monitor_t em);

void set_svr_center_apply_evt(struct set_svr_center * center, enum set_svr_center_fsm_evt_type type);
void set_svr_center_apply_pkg(struct set_svr_center * center, SVR_CENTER_PKG * pkg);

void set_svr_center_link_node_r(set_svr_center_t center, ringbuffer_block_t blk);
void set_svr_center_link_node_w(set_svr_center_t center, ringbuffer_block_t blk);
void set_svr_center_start_watch(set_svr_center_t center);

void set_svr_center_rw_cb(EV_P_ ev_io *w, int revents);

SVR_CENTER_PKG * set_svr_center_get_pkg_buff(set_svr_center_t center, size_t capacity);

#endif
