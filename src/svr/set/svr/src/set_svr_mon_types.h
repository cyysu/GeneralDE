#ifndef SVR_SET_SVR_TYPES_MON_H
#define SVR_SET_SVR_TYPES_MON_H
#include <unistd.h>
#include "set_svr_types.h"

typedef TAILQ_HEAD(set_svr_mon_app_list, set_svr_mon_app) set_svr_mon_app_list_t;

struct set_svr_mon {
    set_svr_t m_svr;

    fsm_def_machine_t m_fsm_def;

    set_svr_mon_app_list_t m_mon_apps;
};

typedef enum set_svr_mon_app_state {
    set_svr_mon_app_state_disable
    , set_svr_mon_app_state_runing
    , set_svr_mon_app_state_waiting
    , set_svr_mon_app_state_checking
} set_svr_mon_app_state_t;

enum set_svr_mon_app_fsm_evt_type {
    set_svr_mon_app_fsm_evt_enable
    , set_svr_mon_app_fsm_evt_disable
    , set_svr_mon_app_fsm_evt_start
    , set_svr_mon_app_fsm_evt_stoped
    , set_svr_mon_app_fsm_evt_timeout
};

struct set_svr_mon_app_fsm_evt {
    enum set_svr_mon_app_fsm_evt_type m_type;
};


struct set_svr_mon_app {
    set_svr_mon_t m_mon;
    set_svr_svr_type_t m_svr_type;
    char * m_bin;
    char * m_pidfile;
    char ** m_args;
    size_t m_arg_count;
    size_t m_arg_capacity;

    struct fsm_machine m_fsm;
    gd_timer_id_t m_fsm_timer_id;
    pid_t m_pid;

    TAILQ_ENTRY(set_svr_mon_app) m_next;
};

#endif
