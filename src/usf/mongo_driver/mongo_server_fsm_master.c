#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_internal_ops.h"

static void mongo_server_fsm_master_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_driver_t driver = server->m_driver;

    if (driver->m_master_server) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: is master, replace old master %s %d!",
            mongo_driver_name(driver), server->m_ip, server->m_port, driver->m_master_server->m_ip, driver->m_master_server->m_port);
    }
    else {
        if (driver->m_debug) {
            CPE_INFO(
                driver->m_em, "%s: server %s %d: is master, set to driver!",
                mongo_driver_name(driver), server->m_ip, server->m_port);
        }
    }

    driver->m_master_server = server;
    driver->m_state = mongo_driver_state_connected;
    mongo_driver_check_update_state(driver);
}

static void mongo_server_fsm_master_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_driver_t driver = server->m_driver;

    assert(driver->m_master_server == server);
    driver->m_master_server = NULL;
}

static uint32_t mongo_server_fsm_master_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct mongo_server_fsm_evt * evt = input_evt;
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_driver_t driver = server->m_driver;

    switch(evt->m_type) {
    case mongo_server_fsm_evt_start:
        return mongo_server_state_connecting;
    case mongo_server_fsm_evt_stop:
        return mongo_server_state_disable;
    case mongo_server_fsm_evt_disconnected:
        return mongo_server_state_connecting;
    case mongo_server_fsm_evt_wb_update:
        ev_io_stop(driver->m_ev_loop, &server->m_watcher);
        mongo_server_start_watch(server);
        return FSM_KEEP_STATE;
    case mongo_server_fsm_evt_recv_pkg:
        if (dp_dispatch_by_string(driver->m_incoming_send_to, mongo_pkg_to_dp_req(evt->m_pkg), driver->m_em) != 0) {
            CPE_ERROR(
                driver->m_em, "%s: server %s %d: on read: dispatch to %s fail!",
                mongo_driver_name(driver), server->m_ip, server->m_port, cpe_hs_data(driver->m_incoming_send_to));
        }
        return FSM_KEEP_STATE;

    default:
        return FSM_INVALID_STATE;
    }
}

int mongo_server_fsm_create_master(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "master", mongo_server_state_master);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_master: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, mongo_server_fsm_master_enter, mongo_server_fsm_master_leave);

    if (fsm_def_state_add_transition(s, mongo_server_fsm_master_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_master: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

