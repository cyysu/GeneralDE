#ifndef SVR_CENTER_AGENT_INTERNAL_OPS_H
#define SVR_CENTER_AGENT_INTERNAL_OPS_H
#include "center_agent_internal_types.h"
#include "protocol/svr/center/svr_center_pro.h"

/*center operations*/
int center_agent_center_init(center_agent_t agent, struct center_agent_center * center);
void center_agent_center_clear(struct center_agent_center * center);
void center_agent_center_connector_state_monitor(net_connector_t connector, void * ctx);
int center_agent_center_ep_init(struct center_agent_center * center, net_ep_t ep);
void center_agent_center_close(struct center_agent_center * center);

int center_agent_center_set_svr(struct center_agent_center * center, const char * ip, short port);
int center_agent_center_set_cvt(struct center_agent_center * center, const char * cvt_name);
int center_agent_center_set_reconnect_span_ms(struct center_agent_center * center, uint32_t span_ms);

int center_agent_center_send(struct center_agent_center * center, SVR_CENTER_PKG * pkg, size_t pkg_size);

/*center fsm operations*/
int center_agent_center_start_state_timer(struct center_agent_center * center, tl_time_span_t span);
void center_agent_center_stop_state_timer(struct center_agent_center * center);

int center_agent_center_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em);
int center_agent_center_fsm_create_join(fsm_def_machine_t fsm_def, error_monitor_t em);
int center_agent_center_fsm_create_idle(fsm_def_machine_t fsm_def, error_monitor_t em);
int center_agent_center_fsm_create_syncing(fsm_def_machine_t fsm_def, error_monitor_t em);

/*svr oprations*/
int center_agent_svr_init(center_agent_t agent, struct center_agent_svr * svr, uint16_t port);
void center_agent_svr_clear(struct center_agent_svr * svr);

int center_agent_svr_set_dispatch_to(struct center_agent_svr * svr, const char * dispatch_to);
void center_agent_svr_set_recv_capacity(struct center_agent_svr * svr, size_t capacity);

/*event operations*/
void center_agent_center_apply_evt(struct center_agent_center * center, enum center_agent_fsm_evt_type type);
void center_agent_center_apply_pkg(struct center_agent_center * center, SVR_CENTER_PKG * pkg);

/*center_svr*/
center_agent_data_svr_t center_agent_data_svr_create(center_agent_t agent, uint16_t svr_type, uint16_t svr_id);
void center_agent_data_svr_free(center_agent_data_svr_t svr);
void center_agent_data_svr_free_all(center_agent_t agent);
uint32_t center_agent_data_svr_hash(center_agent_data_svr_t svr);
int center_agent_data_svr_eq(center_agent_data_svr_t l, center_agent_data_svr_t r);

void center_agent_data_svr_sync(center_agent_t agent, SVR_CENTER_SVR_INFO const * info);

/*center_agent_data_group*/
center_agent_data_group_t center_agent_data_group_create(center_agent_t agent, uint16_t svr_type);
center_agent_data_group_t center_agent_data_group_find(center_agent_t agent, uint16_t svr_type);
void center_agent_data_group_free(center_agent_data_group_t group);
void center_agent_data_group_free_all(center_agent_t agent);
uint32_t center_agent_data_group_hash(center_agent_data_group_t group);
int center_agent_data_group_eq(center_agent_data_group_t l, center_agent_data_group_t r);

#endif
