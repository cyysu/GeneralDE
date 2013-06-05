#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "mongo_internal_ops.h"

static void mongo_driver_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_mongo_driver = {
    "usf_mongo_driver",
    mongo_driver_clear
};

mongo_driver_t
mongo_driver_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    mongo_driver_t driver;
    nm_node_t driver_node;

    driver_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct mongo_driver));
    if (driver_node == NULL) return NULL;

    driver = (mongo_driver_t)nm_node_data(driver_node);
    bzero(driver, sizeof(struct mongo_driver));

    driver->m_app = app;
    driver->m_alloc = alloc;
    driver->m_em = em;
    driver->m_debug = 0;
    driver->m_state = mongo_driver_state_disable;
    driver->m_ringbuf = NULL;
    driver->m_ev_loop = net_mgr_ev_loop(gd_app_net_mgr(app));

    driver->m_fsm_def = NULL;

    driver->m_pkg_buf_max_size = 4 * 1024;
    driver->m_pkg_buf = NULL;

    driver->m_incoming_send_to = NULL;
    driver->m_outgoing_recv_at = NULL;
 
    driver->m_seed_count = 0;
    TAILQ_INIT(&driver->m_seeds);

    driver->m_server_count = 0;
    TAILQ_INIT(&driver->m_servers);
    driver->m_master_server = NULL;

    driver->m_read_block_size = 2 * 1024;
    driver->m_reconnect_span_s = 1;
    driver->m_op_timeout_ms = 30 * 1000;

    driver->m_fsm_def = mongo_server_create_fsm_def(name, alloc, em);
    if (driver->m_fsm_def == NULL) {
        CPE_ERROR(em, "%s: create: create server fsm fail!", name);
        nm_node_free(driver_node);
        return NULL;
    }

    mem_buffer_init(&driver->m_dump_buffer, driver->m_alloc);

    nm_node_set_type(driver_node, &s_nm_node_type_mongo_driver);

    return driver;
} 

static void mongo_driver_clear(nm_node_t node) {
    mongo_driver_t driver;

    driver = (mongo_driver_t)nm_node_data(node);

    if (driver->m_pkg_buf) {
        mongo_pkg_free(driver->m_pkg_buf);
        driver->m_pkg_buf = NULL;
    }

    if (driver->m_incoming_send_to) {
        mem_free(driver->m_alloc, driver->m_incoming_send_to);
        driver->m_incoming_send_to = NULL;
    }

    if (driver->m_outgoing_recv_at) {
        dp_rsp_free(driver->m_outgoing_recv_at);
        driver->m_outgoing_recv_at = NULL;
    }

    mem_buffer_clear(&driver->m_dump_buffer);

    mongo_server_free_all(driver);
    mongo_server_free_all(driver);

    if (driver->m_ringbuf) {
        ringbuffer_delete(driver->m_ringbuf);
        driver->m_ringbuf = NULL;
    }

    if (driver->m_fsm_def) {
        fsm_def_machine_free(driver->m_fsm_def);
        driver->m_fsm_def = NULL;
    }
}

void mongo_driver_free(mongo_driver_t driver) {
    nm_node_t driver_node;
    assert(driver);

    driver_node = nm_node_from_data(driver);
    if (nm_node_type(driver_node) != &s_nm_node_type_mongo_driver) return;
    nm_node_free(driver_node);
}

mongo_driver_t
mongo_driver_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_driver) return NULL;
    return (mongo_driver_t)nm_node_data(node);
}

mongo_driver_t
mongo_driver_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_driver) return NULL;
    return (mongo_driver_t)nm_node_data(node);
}

gd_app_context_t mongo_driver_app(mongo_driver_t driver) {
    return driver->m_app;
}

const char * mongo_driver_name(mongo_driver_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

int mongo_driver_set_incoming_send_to(mongo_driver_t driver, const char * incoming_send_to) {
    size_t name_len = cpe_hs_len_to_binary_len(strlen(incoming_send_to));
    cpe_hash_string_t buf;

    buf = mem_alloc(driver->m_alloc, name_len);
    if (buf == NULL) return -1;

    cpe_hs_init(buf, name_len, incoming_send_to);

    if (driver->m_incoming_send_to) mem_free(driver->m_alloc, driver->m_incoming_send_to);

    driver->m_incoming_send_to = buf;

    return 0;
}

int mongo_driver_set_outgoing_recv_at(mongo_driver_t driver, const char * outgoing_recv_at) {
    char name_buf[128];

    snprintf(name_buf, sizeof(name_buf), "%s.outgoing-recv-rsp", mongo_driver_name(driver));

    if (driver->m_outgoing_recv_at) dp_rsp_free(driver->m_outgoing_recv_at);

    driver->m_outgoing_recv_at = dp_rsp_create(gd_app_dp_mgr(driver->m_app), name_buf);
    if (driver->m_outgoing_recv_at == NULL) return -1;

    dp_rsp_set_processor(driver->m_outgoing_recv_at, mongo_driver_send, driver);

    if (dp_rsp_bind_string(driver->m_outgoing_recv_at, outgoing_recv_at, driver->m_em) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: mongo_driver_set_outgoing_recv_at: bing to %s fail!",
            mongo_driver_name(driver), outgoing_recv_at);
        dp_rsp_free(driver->m_outgoing_recv_at);
        driver->m_outgoing_recv_at = NULL;
        return -1;
    }

    return 0;
}

cpe_hash_string_t
mongo_driver_name_hs(mongo_driver_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

mongo_pkg_t mongo_driver_pkg_buf(mongo_driver_t driver) {
    if (driver->m_pkg_buf) {
        if (mongo_pkg_capacity(driver->m_pkg_buf) < driver->m_pkg_buf_max_size) {
            mongo_pkg_free(driver->m_pkg_buf);
            driver->m_pkg_buf = NULL;
        }
    }

    if (driver->m_pkg_buf == NULL) {
        driver->m_pkg_buf = mongo_pkg_create(driver, driver->m_pkg_buf_max_size);
    }

    mongo_pkg_init(driver->m_pkg_buf);

    return driver->m_pkg_buf;
}

int mongo_driver_check_update_state(mongo_driver_t driver) {
    if (TAILQ_EMPTY(&driver->m_seeds) && TAILQ_EMPTY(&driver->m_servers)) {
        APP_CTX_ERROR(
            driver->m_app, "%s: check connect: no any seed or server, state chanted to error!",
            mongo_driver_name(driver));
        driver->m_state = mongo_driver_state_error;
        return -1;
    }

    if (driver->m_master_server) {
        if (driver->m_state != mongo_driver_state_connected) {
            CPE_INFO(
                driver->m_em, "%s: check connect: connect success, master server: %s:%d!",
                mongo_driver_name(driver), driver->m_master_server->m_ip, driver->m_master_server->m_port);
            driver->m_state = mongo_driver_state_connected;
        }
    }
    else {
        if (driver->m_state != mongo_driver_state_connecting) {
            CPE_INFO(driver->m_em, "%s: check connect: begin connecting!", mongo_driver_name(driver));
            driver->m_state = mongo_driver_state_connecting;
        }
    }

    return 0;
}

int mongo_driver_enable(mongo_driver_t driver) {
    struct mongo_server * seed;
    struct mongo_server * server;

    switch(driver->m_state) {
    case mongo_driver_state_disable:
        break;
    case mongo_driver_state_connecting:
        if (driver->m_debug) {
            CPE_INFO(
                driver->m_em, "%s: enable: is already connecting!",
                mongo_driver_name(driver));
            return 0;
        }
        break;
    case mongo_driver_state_connected:
        if (driver->m_debug) {
            CPE_INFO(
                driver->m_em, "%s: enable: is already connected!",
                mongo_driver_name(driver));
            return 0;
        }
    case mongo_driver_state_error:
        break;
    }

    driver->m_state = mongo_driver_state_connecting;

    TAILQ_FOREACH(seed, &driver->m_seeds, m_next) {
        mongo_server_fsm_apply_evt(seed, mongo_server_fsm_evt_start);
    }

    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        mongo_server_fsm_apply_evt(server, mongo_server_fsm_evt_start);
    }

    return mongo_driver_check_update_state(driver);
}

uint32_t mongo_driver_cur_time_s(mongo_driver_t driver) {
    return (uint32_t) tl_manage_time(gd_app_tl_mgr(driver->m_app)) / 1000;
}

int mongo_driver_add_server(mongo_driver_t driver, const char * host, int port) {
    return mongo_server_create(driver, host, port, mongo_server_runing_mode_server) == NULL ? -1 : 0;
}

int mongo_driver_add_seed(mongo_driver_t driver, const char * host, int port) {
    return mongo_server_create(driver, host, port, mongo_server_runing_mode_seed) == NULL ? -1 : 0;
}

int mongo_driver_set_ringbuf_size(mongo_driver_t driver, size_t capacity) {
    //TODO: disconnect all :)

    if (driver->m_ringbuf) {
        ringbuffer_delete(driver->m_ringbuf);
    }
    driver->m_ringbuf = ringbuffer_new(capacity);

    if (driver->m_ringbuf == NULL) return -1;

    return 0;
}

