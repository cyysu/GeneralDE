#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/buffer.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_internal_ops.h"

mongo_server_t mongo_server_create(mongo_driver_t driver, const char * host, int port, enum mongo_server_runing_mode mode) {
    struct mongo_server * server;

    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        if (strcmp(server->m_ip, host) == 0 && server->m_port == port) {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: create: duplicate!",
                mongo_driver_name(driver), host, port);
            return NULL;
        }
    }

    server = mem_alloc(driver->m_alloc, sizeof(struct mongo_server));
    if (server == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: create: alloc fail!",
            mongo_driver_name(driver), host, port);
        return NULL;
    }

    server->m_driver = driver;
    strncpy(server->m_ip, host, sizeof(server->m_ip));
    server->m_port = port;
    server->m_mode = mode;

    server->m_rb = NULL;
    server->m_wb = NULL;
    server->m_fd = -1;
    server->m_watcher.data = server;

    server->m_max_bson_size = MONGO_DEFAULT_MAX_BSON_SIZE;
    server->m_fsm_timer_id = GD_TIMER_ID_INVALID;

    if (fsm_machine_init(&server->m_fsm, driver->m_fsm_def, "disable", server, driver->m_debug) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: init fsm fail!",
            mongo_driver_name(server->m_driver), host, port);
        mem_free(driver->m_alloc, server);
        return NULL;
    }

    if (mode == mongo_server_runing_mode_server) {
        TAILQ_INSERT_TAIL(&driver->m_servers, server, m_next);
        ++driver->m_server_count;
    }
    else {
        assert(mode == mongo_server_runing_mode_seed);
        TAILQ_INSERT_TAIL(&driver->m_seeds, server, m_next);
        ++driver->m_seed_count;
    }

    return server;
}

void mongo_server_free(struct mongo_server * server) {
    mongo_driver_t driver = server->m_driver;

    mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_stop);
    fsm_machine_fini(&server->m_fsm);

    assert(server->m_fsm_timer_id == GD_TIMER_ID_INVALID);
    assert(server->m_fd == -1);

    if (server->m_rb) {
        ringbuffer_free(driver->m_ringbuf, server->m_rb);
        server->m_rb = NULL;
    }

    if (server->m_wb) {
        ringbuffer_free(driver->m_ringbuf, server->m_wb);
        server->m_wb = NULL;
    }

    if (server->m_mode == mongo_server_runing_mode_server) {
        TAILQ_REMOVE(&driver->m_servers, server, m_next);
        --driver->m_server_count;
    }
    else {
        assert(server->m_mode == mongo_server_runing_mode_seed);
        TAILQ_REMOVE(&driver->m_seeds, server, m_next);
        --driver->m_seed_count;
    }

    mem_free(driver->m_alloc, server);
}

void mongo_server_free_all(mongo_driver_t driver) {
    while(!TAILQ_EMPTY(&driver->m_servers)) {
        mongo_server_free(TAILQ_FIRST(&driver->m_servers));
    }
}

mongo_server_t mongo_server_find_by_fd(mongo_driver_t driver, int fd) {
    mongo_server_t server;

    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        if (server->m_fd == fd) return server;
    }

    TAILQ_FOREACH(server, &driver->m_seeds, m_next) {
        if (server->m_fd == fd) return server;
    }

    return NULL;
}

void mongo_server_fsm_apply_evt(struct mongo_server * server, enum mongo_server_fsm_evt_type type) {
    struct mongo_server_fsm_evt evt;
    evt.m_type = type;
    evt.m_pkg = NULL;
    fsm_machine_apply_event(&server->m_fsm, &evt);
}

void mongo_server_fsm_apply_recv_pkg(struct mongo_server * server, mongo_pkg_t pkg) {
    struct mongo_server_fsm_evt evt;
    evt.m_type = mongo_server_fsm_evt_recv_pkg;
    evt.m_pkg = pkg;
    fsm_machine_apply_event(&server->m_fsm, &evt);
}

static void mongo_server_state_timeout(void * ctx, gd_timer_id_t timer_id, void * arg) {
    struct mongo_server * server = ctx;
    assert(server->m_fsm_timer_id == timer_id);
    mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_timeout);
}

int mongo_server_start_state_timer(struct mongo_server * server, tl_time_span_t span) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(server->m_driver->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(
            server->m_driver->m_em, "%s: start state timer: get default timer manager fail!",
            mongo_driver_name(server->m_driver));
        return -1;
    }

    assert(server->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    if (gd_timer_mgr_regist_timer(timer_mgr, &server->m_fsm_timer_id, mongo_server_state_timeout, server, NULL, NULL, span, span, -1) != 0) {
        assert(server->m_fsm_timer_id == GD_TIMER_ID_INVALID);
        CPE_ERROR(server->m_driver->m_em, "%s: start state timer: regist timer fail!", mongo_driver_name(server->m_driver));
        return -1;
    }

    assert(server->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    return 0;
}

void mongo_server_stop_state_timer(struct mongo_server * server) {
    gd_timer_mgr_t timer_mgr;
    if (server->m_fsm_timer_id == GD_TIMER_ID_INVALID) return;

    timer_mgr = gd_timer_mgr_default(server->m_driver->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(server->m_driver->m_em, "%s: start state timer: get default timer manager fail!", mongo_driver_name(server->m_driver));
        return;
    }

    assert(server->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    gd_timer_mgr_unregist_timer_by_id(timer_mgr, server->m_fsm_timer_id);
    server->m_fsm_timer_id = GD_TIMER_ID_INVALID;
}

static void mongo_server_dump_event(write_stream_t s, fsm_def_machine_t m, void * input_event) {
    struct mongo_server_fsm_evt * evt = input_event;

    switch(evt->m_type) {
    case mongo_server_fsm_evt_start:
        stream_printf(s, "server start");
        break;
    case mongo_server_fsm_evt_stop:
        stream_printf(s, "server stop");
        break;
    case mongo_server_fsm_evt_connected:
        stream_printf(s, "server connected");
        break;
    case mongo_server_fsm_evt_disconnected:
        stream_printf(s, "server disconnected");
        break;
    case mongo_server_fsm_evt_timeout:
        stream_printf(s, "server timeout");
        break;
    case mongo_server_fsm_evt_recv_pkg:
        stream_printf(s, "server recv");
        break;
    case mongo_server_fsm_evt_wb_update:
        stream_printf(s, "write-buf update");
        break;
    default:
        stream_printf(s, "unknown server fsm evt %d", evt->m_type);
        break;
    }
}

fsm_def_machine_t mongo_server_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em) {
    char buf[128];
    fsm_def_machine_t fsm_def;

    snprintf(buf, sizeof(buf), "%s.server", name);
    fsm_def = fsm_def_machine_create(buf, alloc, em);
    if (fsm_def == NULL) {
        CPE_ERROR(em, "mongo_server_create_fsm_def: create fsm def fail!");
        return NULL;
    }

    fsm_def_machine_set_evt_dumper(fsm_def, mongo_server_dump_event);

    if (mongo_server_fsm_create_disable(fsm_def, em) != 0
        || mongo_server_fsm_create_disconnected(fsm_def, em) != 0
        || mongo_server_fsm_create_connecting(fsm_def, em) != 0
        || mongo_server_fsm_create_checking_is_master(fsm_def, em) != 0
        || mongo_server_fsm_create_master(fsm_def, em) != 0
        || mongo_server_fsm_create_slave(fsm_def, em) != 0
        )
    {
        CPE_ERROR(em, "mongo_server_create_fsm_def: init fsm fail!");
        fsm_def_machine_free(fsm_def);
        return NULL;
    }

    return fsm_def;
}

void mongo_server_disconnect(struct mongo_server * server) {
    mongo_driver_t driver = server->m_driver;

    if (server->m_fd == -1) return;

    ev_io_stop(server->m_driver->m_ev_loop, &server->m_watcher);
    cpe_sock_close(server->m_fd);
    server->m_fd = -1;

    if (server->m_rb) {
        ringbuffer_free(driver->m_ringbuf, server->m_rb);
        server->m_rb = NULL;
    }
}

void mongo_server_link_node_r(mongo_server_t server, ringbuffer_block_t blk) {
    if (server->m_rb) {
		ringbuffer_link(server->m_driver->m_ringbuf, server->m_rb , blk);
	}
    else {
		blk->id = 1;
		server->m_rb = blk;
	}
}

void mongo_server_link_node_w(mongo_server_t server, ringbuffer_block_t blk) {
    if (server->m_wb) {
		ringbuffer_link(server->m_driver->m_ringbuf, server->m_wb , blk);
	}
    else {
		blk->id = 2;
		server->m_wb = blk;
	}
}

int mongo_server_alloc(ringbuffer_block_t * result, mongo_driver_t driver, mongo_server_t server, size_t size) {
    ringbuffer_block_t blk;

    blk = ringbuffer_alloc(driver->m_ringbuf , size);
    while (blk == NULL) {
        mongo_server_t disable_server;
        int collect_id = ringbuffer_collect(driver->m_ringbuf);
        if(collect_id < 0) {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: alloc: not enouth capacity, len=%d!",
                mongo_driver_name(driver), server->m_ip, server->m_port, (int)size);
            mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_disconnected);
            return -1;
        }

        disable_server = mongo_server_find_by_fd(driver, collect_id);
        assert(disable_server);

        CPE_INFO(
            driver->m_em, "%s: server %s.%d: alloc: not enouth free buff, disable server %s.%d!",
            mongo_driver_name(driver), server->m_ip, server->m_port, disable_server->m_ip, disable_server->m_port);
        mongo_server_fsm_apply_evt(disable_server, mongo_server_fsm_evt_disconnected);
        if (disable_server == server) return -1;

        blk = ringbuffer_alloc(driver->m_ringbuf , size);
    }

    *result = blk;
    return 0;
}

void mongo_server_start_watch(mongo_server_t server) {
    ev_io_init(&server->m_watcher, mongo_server_rw_cb, server->m_fd, server->m_wb ? (EV_READ | EV_WRITE) : EV_READ);
    ev_io_start(server->m_driver->m_ev_loop, &server->m_watcher);
}

