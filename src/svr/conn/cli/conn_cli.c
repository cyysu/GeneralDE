#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/conn/cli/conn_cli.h"
#include "svr/conn/cli/conn_cli_pkg.h"
#include "conn_cli_internal_ops.h"

struct nm_node_type s_nm_node_type_conn_cli;
static fsm_def_machine_t conn_cli_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em);

conn_cli_t
conn_cli_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    conn_cli_t cli;
    nm_node_t cli_node;

    cli_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct conn_cli));
    if (cli_node == NULL) return NULL;

    cli = (conn_cli_t)nm_node_data(cli_node);
    cli->m_alloc = alloc;
    cli->m_app = app;
    cli->m_em = em;
    cli->m_debug = 0;
    cli->m_ringbuf = NULL;
    cli->m_rb = NULL;
    cli->m_wb = NULL;
    cli->m_tb = NULL;
    cli->m_max_pkg_size = 5 * 1024 * 1025;
    cli->m_reconnect_span_ms = 30 * 1000;
    cli->m_incoming_pkg = NULL;
    cli->m_incoming_body = NULL;
    cli->m_fsm_timer_id = GD_TIMER_ID_INVALID;
    cli->m_fd = -1;
    cli->m_watcher.data = cli;
    cli->m_ev_loop = net_mgr_ev_loop(gd_app_net_mgr(app));
    cli->m_read_block_size = 2048;

    cli->m_fsm_def = conn_cli_create_fsm_def(conn_cli_name(cli), cli->m_alloc, cli->m_em);
    if (cli->m_fsm_def == NULL) {
        CPE_ERROR(cli->m_em, "%s: create fsm def fail!", name);
        nm_node_free(cli_node);
        return NULL;
    }

    if (fsm_machine_init(&cli->m_fsm, cli->m_fsm_def, "disable", cli, 1) != 0) {
        CPE_ERROR(cli->m_em, "%s: init fsm fail!", name);
        fsm_def_machine_free(cli->m_fsm_def);
        nm_node_free(cli_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &cli->m_svrs,
            alloc,
            (cpe_hash_fun_t) conn_cli_svr_stub_hash,
            (cpe_hash_cmp_t) conn_cli_svr_stub_eq,
            CPE_HASH_OBJ2ENTRY(conn_cli_svr_stub, m_hh),
            -1) != 0)
    {
        CPE_ERROR(cli->m_em, "%s: svrs hash table fail!", name);
        fsm_machine_fini(&cli->m_fsm);
        fsm_def_machine_free(cli->m_fsm_def);
        nm_node_free(cli_node);
        return NULL;
    }

    mem_buffer_init(&cli->m_dump_buffer, cli->m_alloc);

    nm_node_set_type(cli_node, &s_nm_node_type_conn_cli);

    return cli;
}

static void conn_cli_clear(nm_node_t node) {
    conn_cli_t cli;

    cli = nm_node_data(node);

    fsm_machine_fini(&cli->m_fsm);
    assert(cli->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    fsm_def_machine_free(cli->m_fsm_def);
    cli->m_fsm_def = NULL;

    conn_cli_svr_stub_free_all(cli);

    if (cli->m_incoming_pkg) {
        conn_cli_pkg_free(cli->m_incoming_pkg);
        cli->m_incoming_pkg = NULL;
    }

    if (cli->m_incoming_body) {
        dp_req_free(cli->m_incoming_body);
        cli->m_incoming_body = NULL;
    }

    if (cli->m_ringbuf) {
        ringbuffer_delete(cli->m_ringbuf);
        cli->m_rb = NULL;
        cli->m_wb = NULL;
        cli->m_tb = NULL;
        cli->m_ringbuf = NULL;
    }

    mem_buffer_clear(&cli->m_dump_buffer);
}

void conn_cli_free(conn_cli_t cli) {
    nm_node_t cli_node;
    assert(cli);

    cli_node = nm_node_from_data(cli);
    if (nm_node_type(cli_node) != &s_nm_node_type_conn_cli) return;
    nm_node_free(cli_node);
}

conn_cli_t
conn_cli_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_conn_cli) return NULL;
    return (conn_cli_t)nm_node_data(node);
}

conn_cli_t
conn_cli_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_conn_cli) return NULL;
    return (conn_cli_t)nm_node_data(node);
}

gd_app_context_t conn_cli_app(conn_cli_t cli) {
    return cli->m_app;
}

const char * conn_cli_name(conn_cli_t cli) {
    return nm_node_name(nm_node_from_data(cli));
}

cpe_hash_string_t
conn_cli_name_hs(conn_cli_t cli) {
    return nm_node_name_hs(nm_node_from_data(cli));
}

void conn_cli_apply_evt(conn_cli_t cli, enum conn_cli_fsm_evt_type type) {
    struct conn_cli_fsm_evt evt;
    evt.m_type = type;
    fsm_machine_apply_event(&cli->m_fsm, &evt);
}

conn_cli_state_t conn_cli_state(conn_cli_t cli) {
    return fsm_machine_curent_state(&cli->m_fsm);
}

void conn_cli_enable(conn_cli_t cli) {
    conn_cli_apply_evt(cli, conn_cli_fsm_evt_start);
}

void conn_cli_disable(conn_cli_t cli) {
    conn_cli_apply_evt(cli, conn_cli_fsm_evt_stop);
}

int conn_cli_set_ringbuf_size(conn_cli_t cli, size_t capacity) {
    int need_start = 0;

    if (conn_cli_state(cli) != conn_cli_state_disable) {
        conn_cli_disable(cli);
        need_start = 1;
    }

    if (cli->m_ringbuf) {
        ringbuffer_delete(cli->m_ringbuf);
        cli->m_rb = NULL;
        cli->m_wb = NULL;
        cli->m_tb = NULL;
    }
    cli->m_ringbuf = ringbuffer_new(capacity);

    if (cli->m_ringbuf == NULL) return -1;

    if (need_start) conn_cli_enable(cli);

    return 0;
}

int conn_cli_set_svr(conn_cli_t cli, const char * ip, uint16_t port) {
    int need_start = 0;

    if (conn_cli_state(cli) == conn_cli_state_disable) {
        conn_cli_disable(cli);
        need_start = 1;
    }

    strncpy(cli->m_ip, ip, sizeof(cli->m_ip));
    cli->m_port = port;

    if (need_start) conn_cli_enable(cli);

    return 0;
}

const char * conn_cli_svr_ip(conn_cli_t cli) {
    return cli->m_ip;
}

uint16_t conn_cli_svr_port(conn_cli_t cli) {
    return cli->m_port;
}

conn_cli_pkg_t conn_cli_incoming_pkg(conn_cli_t cli) {
    if (cli->m_incoming_pkg == NULL) {

        cli->m_incoming_pkg = conn_cli_pkg_create(cli);
        if (cli->m_incoming_pkg == NULL) return NULL;

        cli->m_incoming_body = dp_req_create(gd_app_dp_mgr(cli->m_app), 0);
        if (cli->m_incoming_body == NULL) {
            conn_cli_pkg_free(cli->m_incoming_pkg);
            cli->m_incoming_pkg = NULL;
            return NULL;
        }

        dp_req_set_parent(cli->m_incoming_body, conn_cli_pkg_to_dp_req(cli->m_incoming_pkg));
    }

    return cli->m_incoming_pkg;
}

static void conn_cli_dump_event(write_stream_t s, fsm_def_machine_t m, void * input_event) {
    struct conn_cli_fsm_evt * evt = input_event;
    switch(evt->m_type) {
    case conn_cli_fsm_evt_start:
        stream_printf(s, "start");
        break;
    case conn_cli_fsm_evt_stop:
        stream_printf(s, "stop");
        break;
    case conn_cli_fsm_evt_timeout:
        stream_printf(s, "timeout");
        break;
    case conn_cli_fsm_evt_connected:
        stream_printf(s, "connected");
        break;
    case conn_cli_fsm_evt_disconnected:
        stream_printf(s, "disconnected");
        break;
    }
}


static fsm_def_machine_t
conn_cli_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em) {
    fsm_def_machine_t fsm_def = fsm_def_machine_create(name, alloc, em);
    if (fsm_def == NULL) {
        CPE_ERROR(em, "conn_cli_create_fsm_def: create fsm def fail!");
        return NULL;
    }

    fsm_def_machine_set_evt_dumper(fsm_def, conn_cli_dump_event);

    if (conn_cli_fsm_create_disable(fsm_def, em) != 0
        || conn_cli_fsm_create_connecting(fsm_def, em) != 0
        || conn_cli_fsm_create_disconnected(fsm_def, em) != 0
        || conn_cli_fsm_create_established(fsm_def, em) != 0)
    {
        CPE_ERROR(em, "conn_cli_create_fsm_def: init fsm fail!");
        fsm_def_machine_free(fsm_def);
        return NULL;
    }

    return fsm_def;
}

void conn_cli_disconnect(conn_cli_t cli) {
    if (cli->m_fd == -1) return;

    ev_io_stop(cli->m_ev_loop, &cli->m_watcher);
    cpe_sock_close(cli->m_fd);
    cli->m_fd = -1;

    if (cli->m_rb) {
        ringbuffer_free(cli->m_ringbuf, cli->m_rb);
        cli->m_rb = NULL;
    }
}

static void conn_cli_state_timeout(void * ctx, gd_timer_id_t timer_id, void * arg) {
    conn_cli_t cli = ctx;
    assert(cli->m_fsm_timer_id == timer_id);
    conn_cli_apply_evt(cli, conn_cli_fsm_evt_timeout);
}

int conn_cli_start_state_timer(conn_cli_t cli, tl_time_span_t span) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(cli->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: start state timer: get default timer manager fail!",
            conn_cli_name(cli));
        return -1;
    }

    assert(cli->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    if (gd_timer_mgr_regist_timer(timer_mgr, &cli->m_fsm_timer_id, conn_cli_state_timeout, cli, NULL, NULL, span, span, -1) != 0) {
        assert(cli->m_fsm_timer_id == GD_TIMER_ID_INVALID);
        CPE_ERROR(cli->m_em, "%s: start state timer: regist timer fail!", conn_cli_name(cli));
        return -1;
    }

    assert(cli->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    return 0;
}

void conn_cli_stop_state_timer(conn_cli_t cli) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(cli->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(cli->m_em, "%s: start state timer: get default timer manager fail!", conn_cli_name(cli));
        return;
    }

    assert(cli->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    gd_timer_mgr_unregist_timer_by_id(timer_mgr, cli->m_fsm_timer_id);
    cli->m_fsm_timer_id = GD_TIMER_ID_INVALID;
}

void conn_cli_link_node_r(conn_cli_t cli, ringbuffer_block_t blk) {
    if (cli->m_rb) {
		ringbuffer_link(cli->m_ringbuf, cli->m_rb , blk);
	}
    else {
		blk->id = 1;
		cli->m_rb = blk;
	}
}

void conn_cli_link_node_w(conn_cli_t cli, ringbuffer_block_t blk) {
    if (cli->m_wb) {
		ringbuffer_link(cli->m_ringbuf, cli->m_wb , blk);
	}
    else {
		blk->id = 2;
		cli->m_wb = blk;
	}
}

void conn_cli_start_watch(conn_cli_t cli) {
    ev_io_init(&cli->m_watcher, conn_cli_rw_cb, cli->m_fd, cli->m_wb ? (EV_READ | EV_WRITE) : EV_READ);
    ev_io_start(cli->m_ev_loop, &cli->m_watcher);
}

struct nm_node_type s_nm_node_type_conn_cli = {
    "svr_conn_cli",
    conn_cli_clear
};
