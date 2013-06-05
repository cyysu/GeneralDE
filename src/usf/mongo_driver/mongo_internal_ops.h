#ifndef USF_MONGO_DRIVER_INTERNAL_OPS_H
#define USF_MONGO_DRIVER_INTERNAL_OPS_H
#include "mongo_internal_types.h"

/*driver ops*/
uint32_t mongo_driver_cur_time_s(mongo_driver_t driver);

/*source info ops*/
struct mongo_source_info *
mongo_source_info_create(
    mongo_driver_t driver,
    int32_t source,
    const char * incoming_dispatch_to,
    const char * outgoing_recv_at);

uint32_t mongo_source_info_hash(const struct mongo_source_info * binding);
int mongo_source_info_eq(const struct mongo_source_info * l, const struct mongo_source_info * r);
void mongo_source_info_free_all(mongo_driver_t driver);
struct mongo_source_info * mongo_source_info_find(mongo_driver_t driver, int32_t source);

/*server ops*/
mongo_server_t mongo_server_create(mongo_driver_t driver, const char * host, int port, enum mongo_server_runing_mode mode);
void mongo_server_free_all(mongo_driver_t driver);
fsm_def_machine_t mongo_server_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em);

void mongo_server_disconnect(struct mongo_server * server);
mongo_server_t mongo_server_find_by_fd(mongo_driver_t driver, int fd);
void mongo_server_link_node_r(mongo_server_t server, ringbuffer_block_t blk);
void mongo_server_link_node_w(mongo_server_t server, ringbuffer_block_t blk);
void mongo_server_start_watch(mongo_server_t server);

void mongo_server_fsm_apply_evt(struct mongo_server * server, enum mongo_server_fsm_evt_type type);
void mongo_server_fsm_apply_recv_pkg(struct mongo_server * server, mongo_pkg_t pkg);
int mongo_server_start_state_timer(struct mongo_server * server, tl_time_span_t span);
void mongo_server_stop_state_timer(struct mongo_server * server);

int mongo_server_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em);
int mongo_server_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em);
int mongo_server_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em);
int mongo_server_fsm_create_checking_is_master(fsm_def_machine_t fsm_def, error_monitor_t em);
int mongo_server_fsm_create_master(fsm_def_machine_t fsm_def, error_monitor_t em);
int mongo_server_fsm_create_slave(fsm_def_machine_t fsm_def, error_monitor_t em);

int mongo_server_alloc(ringbuffer_block_t * result, mongo_driver_t driver, mongo_server_t server, size_t size);
void mongo_server_rw_cb(EV_P_ ev_io *w, int revents);

/*process ops*/
int mongo_driver_check_update_state(mongo_driver_t driver);
int mongo_driver_send_to_server(mongo_driver_t driver, struct mongo_server *, mongo_pkg_t pkg);
int mongo_driver_send(dp_req_t req, void * ctx, error_monitor_t em);

void mongo_driver_process_internal(mongo_driver_t driver, mongo_pkg_t pkg);

#endif
