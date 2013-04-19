#include <assert.h>
#include "cpe/utils/error.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

static void center_agent_center_fsm_idle_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
}

static uint32_t center_agent_center_fsm_idle_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    return FSM_INVALID_STATE;
}

int center_agent_center_fsm_create_idle(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create(fsm_def, "idle");
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_idle: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, center_agent_center_fsm_idle_enter, NULL);

    if (fsm_def_state_add_transition(s, center_agent_center_fsm_idle_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_idle: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
