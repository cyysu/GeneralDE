#include <assert.h>
#include "cpe/utils/error.h"
#include "set_svr_center_ops.h"

static void set_svr_center_fsm_syncing_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct set_svr_center * center = fsm_machine_context(fsm);
    set_svr_t svr = center->m_svr;
    SVR_CENTER_PKG * pkg;
    SVR_CENTER_REQ_QUERY_SVR_BY_TYPE * query;
    struct cpe_hash_it svr_type_it;
    set_svr_svr_type_t svr_type;
    size_t pkg_capacity;

    set_svr_center_start_state_timer(center, 30000);

    pkg_capacity = sizeof(SVR_CENTER_PKG) + sizeof(uint16_t) * cpe_hash_table_count(&svr->m_svr_types_by_id);
    pkg = set_svr_center_get_pkg_buff(center, pkg_capacity);
    if (pkg == NULL) {
        CPE_ERROR(center->m_svr->m_em, "%s: send syncing: get pkg buf fail!", set_svr_name(center->m_svr));
        return;
    }

    query = &pkg->data.svr_center_req_query_svr_by_type;

    pkg->cmd = SVR_CENTER_CMD_REQ_QUERY_SVR_BY_TYPE;
    query->count = 0;

    cpe_hash_it_init(&svr_type_it, &center->m_svr->m_svr_types_by_id);
    for(svr_type = cpe_hash_it_next(&svr_type_it);
        svr_type;
        svr_type = cpe_hash_it_next(&svr_type_it))
    {
        query->types[query->count++] = svr_type->m_svr_type_id;
    }

    if (set_svr_center_send(center, pkg, pkg_capacity) != 0) {
        CPE_ERROR(center->m_svr->m_em, "%s: send syncing req fail!", set_svr_name(center->m_svr));
    }
    else {
        if (center->m_svr->m_debug) {
            CPE_INFO(center->m_svr->m_em, "%s: send syncing!", set_svr_name(center->m_svr));
        }
    }
}

static void set_svr_center_fsm_syncing_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct set_svr_center * center = fsm_machine_context(fsm);
    set_svr_center_stop_state_timer(center);
}

static uint32_t set_svr_center_fsm_syncing_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_center_fsm_evt * evt = input_evt;
    struct set_svr_center * center = fsm_machine_context(fsm);

    switch(evt->m_type) {
    case set_svr_center_fsm_evt_pkg:
        if (evt->m_pkg->cmd == SVR_CENTER_CMD_RES_QUERY_SVR_BY_TYPE) {
            uint16_t i;
            SVR_CENTER_RES_QUERY_SVR_BY_TYPE const * syncing_res;

            syncing_res = &evt->m_pkg->data.svr_center_res_query_svr_by_type;

            for(i = 0; i < syncing_res->count; ++i) {
                //set_svr_data_svr_sync(center->m_svr, &syncing_res->data[i]);
            }

            return set_svr_center_state_idle;
        }
        else {
            return FSM_INVALID_STATE;
        }
    case set_svr_center_fsm_evt_timeout:
        CPE_ERROR(center->m_svr->m_em, "%s: send syncing: timeout", set_svr_name(center->m_svr));
        return set_svr_center_state_syncing;
    case set_svr_center_fsm_evt_disconnected:
        return set_svr_center_state_connecting;
    case set_svr_center_fsm_evt_wb_update:
        ev_io_stop(center->m_svr->m_ev_loop, &center->m_watcher);
        set_svr_center_start_watch(center);
        return FSM_KEEP_STATE;
    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_center_fsm_create_syncing(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "syncing", set_svr_center_state_syncing);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_syncing: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_center_fsm_syncing_enter, set_svr_center_fsm_syncing_leave);

    if (fsm_def_state_add_transition(s, set_svr_center_fsm_syncing_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_syncing: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
