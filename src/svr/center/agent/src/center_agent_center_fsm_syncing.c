#include <assert.h>
#include "cpe/utils/error.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

static void center_agent_center_fsm_syncing_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct center_agent_center * center = fsm_machine_context(fsm);
    SVR_CENTER_PKG pkg;
    SVR_CENTER_REQ_QUERY_SVR_BY_TYPE * query = &pkg.data.svr_center_req_query_svr_by_type;
    int capacity = sizeof(query->types) / sizeof(query->types[0]);
    struct cpe_hash_it group_it;
    center_agent_data_group_t group;
 
    center_agent_center_start_state_timer(center, 30000);

    pkg.cmd = SVR_CENTER_CMD_REQ_QUERY_SVR_BY_TYPE;
    query->count = 0;
    
    cpe_hash_it_init(&group_it, &center->m_agent->m_groups);
    for(group = cpe_hash_it_next(&group_it);
        query->count < capacity && group;
        group = cpe_hash_it_next(&group_it), ++query->count)
    {
        query->types[query->count] = group->m_svr_type;
    }

    if (center_agent_center_send(center, &pkg, sizeof(pkg)) != 0) {
        CPE_ERROR(center->m_agent->m_em, "%s: send syncing req fail!", center_agent_name(center->m_agent));
    }
    else {
        if (center->m_agent->m_debug) {
            CPE_INFO(center->m_agent->m_em, "%s: send syncing!", center_agent_name(center->m_agent));
        }
    }
}

static void center_agent_center_fsm_syncing_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct center_agent_center * center = fsm_machine_context(fsm);
    center_agent_center_stop_state_timer(center);
}

static uint32_t center_agent_center_fsm_syncing_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct center_agent_fsm_evt * evt = input_evt;
    struct center_agent_center * center = fsm_machine_context(fsm);

    switch(evt->m_type) {
    case center_agent_fsm_evt_pkg:
        if (evt->m_pkg->cmd == SVR_CENTER_CMD_RES_QUERY_SVR_BY_TYPE) {
            uint16_t i;
            SVR_CENTER_RES_QUERY_SVR_BY_TYPE const * syncing_res;

            syncing_res = &evt->m_pkg->data.svr_center_res_query_svr_by_type;

            for(i = 0; i < syncing_res->count; ++i) {
                center_agent_data_svr_sync(center->m_agent, &syncing_res->data[i]);
            }

            return fsm_state_to_id(fsm, "idle");
        }
        else {
            return FSM_INVALID_STATE;
        }
    case center_agent_fsm_evt_timeout:
        CPE_ERROR(center->m_agent->m_em, "%s: send syncing: timeout", center_agent_name(center->m_agent));
        return fsm_state_to_id(fsm, "syncing");
    default:
        return FSM_INVALID_STATE;
    }
}

int center_agent_center_fsm_create_syncing(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create(fsm_def, "syncing");
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_syncing: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, center_agent_center_fsm_syncing_enter, center_agent_center_fsm_syncing_leave);

    if (fsm_def_state_add_transition(s, center_agent_center_fsm_syncing_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_syncing: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
