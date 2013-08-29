#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/error.h"
#include "set_svr_router_ops.h"

void set_svr_router_conn_accepting_rw_cb(EV_P_ ev_io *w, int revents);

static void set_svr_router_conn_fsm_accepting_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_router_conn_t conn = fsm_machine_context(fsm);

    set_svr_router_conn_start_state_timer(conn, 30000);

    ev_io_init(&conn->m_watcher, set_svr_router_conn_accepting_rw_cb, conn->m_fd, EV_READ);
    ev_io_start(conn->m_svr->m_ev_loop, &conn->m_watcher);
}

static void set_svr_router_conn_fsm_accepting_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct set_svr_router_conn * conn = fsm_machine_context(fsm);
    set_svr_router_conn_stop_state_timer(conn);

    ev_io_stop(conn->m_svr->m_ev_loop, &conn->m_watcher);
}

static uint32_t set_svr_router_conn_fsm_accepting_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_router_conn_fsm_evt * evt = input_evt;
    struct set_svr_router_conn * conn = fsm_machine_context(fsm);
    set_svr_t svr = conn->m_svr;

    switch(evt->m_type) {
    case set_svr_router_conn_fsm_evt_timeout:
        CPE_ERROR(svr->m_em, "%s: conn %d: accepting: timeout", set_svr_name(svr), conn->m_fd);
        set_svr_router_conn_free(conn);
        return FSM_DESTORIED_STATE;

    case set_svr_router_conn_fsm_evt_disconnected:
        CPE_ERROR(svr->m_em, "%s: conn %d: accepting: disconnected", set_svr_name(svr), conn->m_fd);
        set_svr_router_conn_free(conn);
        return FSM_DESTORIED_STATE;

    case set_svr_router_conn_fsm_evt_accepted:
        CPE_ERROR(svr->m_em, "%s: conn %d: accepting: accept success", set_svr_name(svr), conn->m_fd);
        return set_svr_router_conn_state_established;

    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_router_conn_fsm_create_accepting(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "accepting", set_svr_router_conn_state_accepting);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_accepting: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_router_conn_fsm_accepting_enter, set_svr_router_conn_fsm_accepting_leave);

    if (fsm_def_state_add_transition(s, set_svr_router_conn_fsm_accepting_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_accepting: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

void set_svr_router_conn_accepting_rw_cb(EV_P_ ev_io *w, int revents) {
    set_svr_router_conn_t conn = w->data;
    set_svr_t svr = conn->m_svr;
    set_svr_router_t router;
    char * buf;
    size_t require_size = sizeof(uint16_t);
    int receive_size;
    uint16_t router_port;
    uint32_t router_ip;
    struct sockaddr_in addr;
    socklen_t addr_len;

    assert(!(revents & EV_WRITE));

    if (!(revents & EV_READ)) return;

    if (set_svr_router_conn_read_from_net(conn, require_size) != 0) {
        set_svr_router_conn_apply_evt(conn, set_svr_router_conn_fsm_evt_disconnected);
        return;
    }

    receive_size = set_svr_router_conn_r_buf(conn, require_size, (void **)&buf);

    if (receive_size < 0) {
        set_svr_router_conn_apply_evt(conn, set_svr_router_conn_fsm_evt_disconnected);
        return;
    }

    if (receive_size < require_size) return;

    assert(buf);

    CPE_COPY_NTOH16(&router_port, buf);

    addr_len = sizeof(addr);
    if (cpe_getpeername(conn->m_fd, (struct sockaddr *)&addr, &addr_len) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: accepting: get peername error, errno=%d (%s)!",
            set_svr_name(svr), conn->m_fd, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        set_svr_router_conn_apply_evt(conn, set_svr_router_conn_fsm_evt_disconnected);
        return;
    }

    router_ip = addr.sin_addr.s_addr;

    router = set_svr_router_find_by_addr(svr, router_ip, router_port);
    if (router == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: accepting: router %d.%d not exist!",
            set_svr_name(svr), conn->m_fd, router_ip, router_port);
        set_svr_router_conn_apply_evt(conn, set_svr_router_conn_fsm_evt_disconnected);
        return;
    }

    if (router->m_conn) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: accepting: router %d-%d.%d already have conn %d!",
            set_svr_name(svr), conn->m_fd, router->m_id, router->m_ip, router->m_port, router->m_conn->m_fd);
        set_svr_router_conn_apply_evt(conn, set_svr_router_conn_fsm_evt_disconnected);
        return;
    }

    set_svr_router_conn_set_router(conn, router);
    set_svr_router_conn_r_erase(conn, require_size);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: conn %d: accepting: bind to router %d-%d.%d!",
            set_svr_name(svr), conn->m_fd, router->m_id, router->m_ip, router->m_port);
    }

    set_svr_router_conn_apply_evt(conn, set_svr_router_conn_fsm_evt_accepted);
}
