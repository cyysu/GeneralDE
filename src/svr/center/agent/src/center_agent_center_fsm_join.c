#include <assert.h>
#include "cpe/utils/error.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

static void center_agent_center_fsm_join_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct center_agent_center * center = fsm_machine_context(fsm);
    SVR_CENTER_PKG pkg;

    center_agent_center_start_state_timer(center, 30000);

    pkg.cmd = SVR_CENTER_CMD_REQ_JOIN;
    pkg.data.svr_center_req_join.id.svr_type = center->m_agent->m_svr_type;
    pkg.data.svr_center_req_join.id.svr_id = center->m_agent->m_svr_id;
    pkg.data.svr_center_req_join.port = center->m_agent->m_svr.m_port;

    if (center_agent_center_send(center, &pkg, sizeof(pkg)) != 0) {
        CPE_ERROR(center->m_agent->m_em, "%s: send join req fail!", center_agent_name(center->m_agent));
        center_agent_center_close(center);
    }
    else {
        if (center->m_agent->m_debug) {
            CPE_INFO(center->m_agent->m_em, "%s: send join!", center_agent_name(center->m_agent));
        }
    }
}

static void center_agent_center_fsm_join_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct center_agent_center * center = fsm_machine_context(fsm);
    center_agent_center_stop_state_timer(center);
}

static uint32_t center_agent_center_fsm_join_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct center_agent_fsm_evt * evt = input_evt;
    struct center_agent_center * center = fsm_machine_context(fsm);

    switch(evt->m_type) {
    case center_agent_fsm_evt_pkg:
        if (evt->m_pkg->cmd == SVR_CENTER_CMD_RES_JOIN) {
            SVR_CENTER_RES_JOIN const * join_res;
            join_res = &evt->m_pkg->data.svr_center_res_join;
            if (join_res->result != 0) {
                CPE_ERROR(center->m_agent->m_em, "%s: send join: error, result=%d", center_agent_name(center->m_agent), join_res->result);
                return fsm_state_to_id(fsm, "disconnected");
            }

            return fsm_state_to_id(fsm, "syncing");
        }
        else {
            return FSM_INVALID_STATE;
        }
    case center_agent_fsm_evt_timeout:
        CPE_ERROR(center->m_agent->m_em, "%s: send join: timeout", center_agent_name(center->m_agent));
        return fsm_state_to_id(fsm, "join");
    default:
        return FSM_INVALID_STATE;
    }
}

int center_agent_center_fsm_create_join(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create(fsm_def, "join");
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_join: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, center_agent_center_fsm_join_enter, center_agent_center_fsm_join_leave);

    if (fsm_def_state_add_transition(s, center_agent_center_fsm_join_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_join: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
