#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "mongo_internal_ops.h"

static void mongo_server_fsm_disconnected_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_driver_t driver = server->m_driver;

    mongo_server_disconnect(server);

    if (driver->m_reconnect_span_s == 0) {
        CPE_ERROR(driver->m_em, "%s: server %s.%d: reconnect time span is 0, stop!", mongo_driver_name(driver), server->m_ip, server->m_port);
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_stop);
        return;
    }

    if (mongo_server_start_state_timer(server, driver->m_reconnect_span_s * 1000) != 0) {
        CPE_ERROR(driver->m_em, "%s: server %s.%d: start timer fail!", mongo_driver_name(driver), server->m_ip, server->m_port);
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_disconnected);
        return;
    }
}

static void mongo_server_fsm_disconnected_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_server_stop_state_timer(server);
}

static uint32_t mongo_server_fsm_disconnected_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct mongo_server_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case mongo_server_fsm_evt_timeout:
    case mongo_server_fsm_evt_start:
        return mongo_server_state_connecting;
    case mongo_server_fsm_evt_stop:
        return mongo_server_state_disable;
    default:
        return FSM_INVALID_STATE;
    }
}

int mongo_server_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "disconnected", mongo_server_state_disconnected);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_disconnected: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, mongo_server_fsm_disconnected_enter, mongo_server_fsm_disconnected_leave);

    if (fsm_def_state_add_transition(s, mongo_server_fsm_disconnected_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_disconnected: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

