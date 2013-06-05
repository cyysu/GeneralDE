#include <assert.h>
#include "cpe/utils/error.h"
#include "svr/conn/cli/conn_cli.h"
#include "conn_cli_internal_ops.h"

static void conn_cli_fsm_established_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct conn_cli * cli = fsm_machine_context(fsm);
    conn_cli_start_watch(cli);
}

static void conn_cli_fsm_established_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct conn_cli * cli = fsm_machine_context(fsm);
    ev_io_stop(cli->m_ev_loop, &cli->m_watcher);
}

static uint32_t conn_cli_fsm_established_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct conn_cli_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case conn_cli_fsm_evt_stop:
        return conn_cli_state_disable;
    case conn_cli_fsm_evt_disconnected:
        return conn_cli_state_connecting;
    default:
        return FSM_INVALID_STATE;
    }
}

int conn_cli_fsm_create_established(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "established", conn_cli_state_established);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_established: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, conn_cli_fsm_established_enter, conn_cli_fsm_established_leave);

    if (fsm_def_state_add_transition(s, conn_cli_fsm_established_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_established: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
