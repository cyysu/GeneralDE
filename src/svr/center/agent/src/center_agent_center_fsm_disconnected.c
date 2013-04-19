#include <assert.h>
#include "cpe/utils/error.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

static void center_agent_center_fsm_disconnected_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct center_agent_center * center = fsm_machine_context(fsm);
    center_agent_center_start_state_timer(center, 30000);
}

static void center_agent_center_fsm_disconnected_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct center_agent_center * center = fsm_machine_context(fsm);
    center_agent_center_stop_state_timer(center);
}

static uint32_t center_agent_center_fsm_disconnected_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct center_agent_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case center_agent_fsm_evt_connected:
    case center_agent_fsm_evt_timeout:
        return fsm_state_to_id(fsm, "join");
    default:
        break;
    }

    return FSM_INVALID_STATE;
}

int center_agent_center_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create(fsm_def, "disconnected");
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_disconnected: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, center_agent_center_fsm_disconnected_enter, center_agent_center_fsm_disconnected_leave);

    if (fsm_def_state_add_transition(s, center_agent_center_fsm_disconnected_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_disconnected: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
