#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_internal_ops.h"

static void mongo_server_fsm_checking_is_master_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_driver_t driver = server->m_driver;
    mongo_pkg_t pkg_buf;

    assert(server->m_fd != -1);

    if (mongo_server_start_state_timer(server, driver->m_op_timeout_ms) != 0) {
        CPE_ERROR(driver->m_em, "%s: server %s.%d: start check-is-master timer fail!", mongo_driver_name(driver), server->m_ip, server->m_port);
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_disconnected);
        return;
    }

    pkg_buf = mongo_driver_pkg_buf(driver);
    if (pkg_buf == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: check is master: get pkg buf fail!",
            mongo_driver_name(driver), server->m_ip, server->m_port);
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_stop);
        return;
    }

    mongo_pkg_cmd_init(pkg_buf);
    mongo_pkg_set_db(pkg_buf, "admin");
    if (mongo_pkg_doc_open(pkg_buf) != 0
        || mongo_pkg_append_int32(pkg_buf, "ismaster", 1) != 0
        || mongo_pkg_doc_close(pkg_buf) != 0
        || mongo_driver_send_to_server(driver, server, pkg_buf))
    {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: check is master: send cmd fail!",
            mongo_driver_name(driver), server->m_ip, server->m_port);
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_stop);
        return;
    }

    if (driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: check is master: send cmd success!",
            mongo_driver_name(driver), server->m_ip, server->m_port);
    }

}

static void mongo_server_fsm_checking_is_master_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_server_stop_state_timer(server);
}

static uint32_t mongo_server_fsm_checking_is_master_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct mongo_server_fsm_evt * evt = input_evt;
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_driver_t driver = server->m_driver;
    bson_iterator it;

    switch(evt->m_type) {
    case mongo_server_fsm_evt_wb_update:
        ev_io_stop(driver->m_ev_loop, &server->m_watcher);
        mongo_server_start_watch(server);
        return FSM_KEEP_STATE;
    case mongo_server_fsm_evt_recv_pkg:
        break;
    case mongo_server_fsm_evt_stop:
        return mongo_server_state_disable;
    case mongo_server_fsm_evt_disconnected:
        return mongo_server_state_connecting;
    case mongo_server_fsm_evt_timeout:
        return mongo_server_state_checking_is_master;
    default:
        return FSM_INVALID_STATE;
    }

    assert(evt->m_pkg);

    mongo_pkg_it(&it, evt->m_pkg, 0);

    if(mongo_pkg_find(&it, evt->m_pkg, 0, "maxBsonObjectSize") == 0) {
        server->m_max_bson_size = bson_iterator_int(&it);
    }
    else {
        server->m_max_bson_size = MONGO_DEFAULT_MAX_BSON_SIZE;
    }

    if(mongo_pkg_find(&it, evt->m_pkg, 0, "ismaster") != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: check is master, no ismaster in result!",
            mongo_driver_name(driver), server->m_ip, server->m_port);
        return mongo_server_state_disconnected;
    }

    return bson_iterator_bool(&it)
        ? mongo_server_state_master
        : mongo_server_state_slave;
}

int mongo_server_fsm_create_checking_is_master(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "checking_is_master", mongo_server_state_checking_is_master);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_checking_is_master: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, mongo_server_fsm_checking_is_master_enter, mongo_server_fsm_checking_is_master_leave);

    if (fsm_def_state_add_transition(s, mongo_server_fsm_checking_is_master_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_checking_is_master: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

