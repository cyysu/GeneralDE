#ifndef SVR_SET_SVR_MON_OPS_H
#define SVR_SET_SVR_MON_OPS_H
#include "set_svr_mon_types.h"
#include "set_svr_ops.h"

/*operations of set_svr_mon*/
set_svr_mon_t set_svr_mon_create(set_svr_t svr);
void set_svr_mon_free(set_svr_mon_t mon);

/*operations of set_svr_mon_app*/
set_svr_mon_app_t set_svr_mon_app_create(
    set_svr_mon_t mon, set_svr_svr_type_t svr_type, const char * bin, const char * pidfile);

void set_svr_mon_app_free(set_svr_mon_app_t mon_app);
set_svr_mon_app_t set_svr_mon_app_find_by_pid(set_svr_mon_t mon, int pid);
const char * set_svr_mon_app_pid_file(set_svr_mon_app_t mon_app, char * buf, size_t buf_capacity);
void set_svr_mon_app_start_all(set_svr_mon_t mon_app);
int set_svr_mon_app_add_arg(set_svr_mon_app_t mon_app, const char * arg);

enum set_svr_mon_app_get_pid_result {
    set_svr_mon_app_get_pid_ok
    , set_svr_mon_app_get_pid_not_runing
    , set_svr_mon_app_get_pid_error
};
enum set_svr_mon_app_get_pid_result set_svr_mon_app_get_pid(set_svr_mon_app_t mon_app, int * pid);
int set_svr_mon_app_kill(set_svr_mon_app_t mon_app, int sig);

/*operations of mon_app fsm*/
void set_svr_mon_app_apply_evt(struct set_svr_mon_app * mon_app, enum set_svr_mon_app_fsm_evt_type type);
int set_svr_mon_app_start_state_timer(struct set_svr_mon_app * mon_app, tl_time_span_t span);
void set_svr_mon_app_stop_state_timer(struct set_svr_mon_app * mon_app);

fsm_def_machine_t set_svr_mon_app_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em);
int set_svr_mon_app_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_mon_app_fsm_create_checking(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_mon_app_fsm_create_runing(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_mon_app_fsm_create_waiting(fsm_def_machine_t fsm_def, error_monitor_t em);

#endif
