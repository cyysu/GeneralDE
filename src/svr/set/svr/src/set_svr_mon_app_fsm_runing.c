#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include "cpe/utils/error.h"
#include "set_svr_mon_ops.h"

static void set_svr_mon_app_fsm_runing_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_mon_app_t mon_app = fsm_machine_context(fsm);
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;
    int pid;

    set_svr_mon_app_start_state_timer(mon_app, 30000);

    pid = fork();
    if (pid < 0) { 
        CPE_ERROR(
            svr->m_em, "%s: mon app %s: start: fork fail, error=%d (%s)!",
            set_svr_name(svr), mon_app->m_bin, errno, strerror(errno));
        set_svr_mon_app_apply_evt(mon_app, set_svr_mon_app_fsm_evt_stoped);
        return;
    }
    else if (pid == 0) { /*子进程 */
        execv(mon_app->m_bin, mon_app->m_args);
        exit(-1);
    }
    else { /*父进程 */
        mon_app->m_pid = pid;

        CPE_INFO(
            svr->m_em, "%s: mon app %s: start: fork success, pid=%d!",
            set_svr_name(svr), mon_app->m_bin, pid);
    }
}

static void set_svr_mon_app_fsm_runing_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_mon_app_t mon_app = fsm_machine_context(fsm);

    set_svr_mon_app_stop_state_timer(mon_app);
}

static uint32_t set_svr_mon_app_fsm_runing_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_mon_app_fsm_evt * evt = input_evt;
    struct set_svr_mon_app * mon_app = fsm_machine_context(fsm);
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;

    switch(evt->m_type) {
    case set_svr_mon_app_fsm_evt_stoped:
        return set_svr_mon_app_state_waiting;
    case set_svr_mon_app_fsm_evt_timeout: {
        int pid;

        switch(set_svr_mon_app_get_pid(mon_app, &pid)) {
        case set_svr_mon_app_get_pid_ok:
            if (mon_app->m_pid != pid) {
                CPE_ERROR(
                    svr->m_em, "%s: mon app %s: check in runing: pid changed %d ==> %d",
                    set_svr_name(svr), mon_app->m_bin, mon_app->m_pid, pid);
                if (mon_app->m_pid) set_svr_mon_app_kill(mon_app, SIGKILL);
                return set_svr_mon_app_state_checking;
            }
            else {
                if (svr->m_debug) {
                    CPE_ERROR(
                        svr->m_em, "%s: mon app %s: check in runing: process %d still runing",
                        set_svr_name(svr), mon_app->m_bin, pid);
                }
                return FSM_KEEP_STATE;
            }
        case set_svr_mon_app_get_pid_not_runing:
            CPE_ERROR(
                svr->m_em, "%s: mon app %s: check in runing: process %d is not runing!",
                set_svr_name(svr), mon_app->m_bin, mon_app->m_pid);
            return set_svr_mon_app_state_waiting;
        case set_svr_mon_app_get_pid_error:
            return FSM_KEEP_STATE;
        }
    }
    case set_svr_mon_app_fsm_evt_disable:
        return set_svr_mon_app_state_disable;
    default:
        printf("********** other\n");
        return FSM_INVALID_STATE;
    }
}

int set_svr_mon_app_fsm_create_runing(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "runing", set_svr_mon_app_state_runing);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_runing: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_mon_app_fsm_runing_enter, set_svr_mon_app_fsm_runing_leave);

    if (fsm_def_state_add_transition(s, set_svr_mon_app_fsm_runing_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_runing: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
