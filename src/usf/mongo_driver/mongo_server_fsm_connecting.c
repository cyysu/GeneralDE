#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_socket.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "mongo_internal_ops.h"

static void mongo_server_connect_cb(EV_P_ ev_io *w, int revents);

static void mongo_server_fsm_connecting_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_driver_t driver = server->m_driver;
    struct sockaddr_in addr;

    if (mongo_server_start_state_timer(server, driver->m_op_timeout_ms) != 0) {
        CPE_ERROR(driver->m_em, "%s: server %s.%d: start timer fail!", mongo_driver_name(driver), server->m_ip, server->m_port);
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_disconnected);
        return;
    }

    mongo_server_disconnect(server);

    assert(server->m_fd == -1);

    server->m_fd = cpe_sock_open(AF_INET, SOCK_STREAM, 0);
    if (server->m_fd == -1) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: create socket fail, errno=%d (%s)!",
            mongo_driver_name(driver), server->m_ip, server->m_port,
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_disconnected);
        return;
    }

    if (cpe_sock_set_none_block(server->m_fd, 1) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: set socket none block fail, errno=%d (%s)!",
            mongo_driver_name(driver), server->m_ip, server->m_port,
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_disconnected);
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(server->m_port);
    addr.sin_addr.s_addr = inet_addr(server->m_ip);

    if (cpe_connect(server->m_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        if (cpe_sock_errno() == EINPROGRESS || cpe_sock_errno() == EWOULDBLOCK) {
            CPE_INFO(
                driver->m_em, "%s: server %s.%d: connect started!",
                mongo_driver_name(driver), server->m_ip, server->m_port);
            ev_io_init(&server->m_watcher, mongo_server_connect_cb, server->m_fd, EV_WRITE);
            ev_io_start(driver->m_ev_loop, &server->m_watcher);
            return;
        }
        else {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: connect error, errno=%d (%s)",
                mongo_driver_name(driver), server->m_ip, server->m_port,
                cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
            cpe_sock_close(server->m_fd);
            server->m_fd = -1;
            mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_disconnected);
            return;
        }
    }
    else {
        CPE_INFO(
            driver->m_em, "%s: server %s.%d: connect succeed!",
            mongo_driver_name(driver), server->m_ip, server->m_port);
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_connected);
        return;
    }
}

static void mongo_server_fsm_connecting_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_server_stop_state_timer(server);
}

static uint32_t mongo_server_fsm_connecting_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct mongo_server_fsm_evt * evt = input_evt;
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_driver_t driver = server->m_driver;

    switch(evt->m_type) {
    case mongo_server_fsm_evt_stop:
        return mongo_server_state_disable;

    case mongo_server_fsm_evt_disconnected:
        return mongo_server_state_disconnected;

    case mongo_server_fsm_evt_connected:
        mongo_server_start_watch(server);
        return mongo_server_state_checking_is_master;

    case mongo_server_fsm_evt_timeout:
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: connecting timeout!",
            mongo_driver_name(driver), server->m_ip, server->m_port);
        return mongo_server_state_connecting;

    default:
        return FSM_INVALID_STATE;
    }
}

int mongo_server_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "connecting", mongo_server_state_connecting);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_connecting: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, mongo_server_fsm_connecting_enter, mongo_server_fsm_connecting_leave);

    if (fsm_def_state_add_transition(s, mongo_server_fsm_connecting_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_connecting: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

static void mongo_server_connect_cb(EV_P_ ev_io *w, int revents) {
    mongo_server_t server = w->data;
    mongo_driver_t driver = server->m_driver;
    int err;
    socklen_t err_len;

    err_len = sizeof(err);

    ev_io_stop(driver->m_ev_loop, &server->m_watcher);

    if (cpe_getsockopt(server->m_fd, SOL_SOCKET, SO_ERROR, &err, &err_len) == -1) {
        CPE_ERROR(
            driver->m_em,
            "%s: server %s.%d: check state, getsockopt error, errno=%d (%s)",
            mongo_driver_name(driver), server->m_ip, server->m_port, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_disconnected);
    }
    else {
        if (err == 0) {
            CPE_INFO(
                driver->m_em, "%s: server %s.%d: connect succeed!",
                mongo_driver_name(driver), server->m_ip, server->m_port);
            mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_connected);
        }
        else {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: connect error, errno=%d (%s)",
                mongo_driver_name(driver), server->m_ip, server->m_port, err, cpe_sock_errstr(err));
            mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_disconnected);
        }
    }
}
