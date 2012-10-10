#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_connector.h"
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
    driver->m_connecting_seed_count = 0;
    driver->m_connecting_server_count = 0;

    driver->m_pkg_buf_max_size = 4 * 1024;
    driver->m_pkg_buf = NULL;

    driver->m_incoming_send_to = NULL;
    driver->m_outgoing_recv_at = NULL;
 
    driver->m_dump_buffer_capacity = 4 * 1024;

    driver->m_seed_count = 0;
    TAILQ_INIT(&driver->m_seeds);

    driver->m_server_count = 0;
    TAILQ_INIT(&driver->m_servers);
    driver->m_master_server = NULL;
    driver->m_server_read_chanel_size = 4 * 1024 * 10;
    driver->m_server_write_chanel_size = 4 * 1024;

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
    mongo_seed_free_all(driver);
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

    snprintf(name_buf, sizeof(name_buf), "%s.outgoing-recv-rsp", outgoing_recv_at);

    if (driver->m_outgoing_recv_at) dp_rsp_free(driver->m_outgoing_recv_at);

    driver->m_outgoing_recv_at = dp_rsp_create(gd_app_dp_mgr(driver->m_app), name_buf);
    if (driver->m_outgoing_recv_at == NULL) return -1;

    dp_rsp_set_processor(driver->m_outgoing_recv_at, mongo_driver_send, driver);

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

int mongo_driver_connect_i(mongo_driver_t driver) {
    struct mongo_server * server;
    struct mongo_seed * seed;

    driver->m_state = mongo_driver_state_connecting;

    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        mongo_server_disable(server);
    }

    driver->m_connecting_seed_count = 0;
    driver->m_connecting_server_count = 0;
    driver->m_master_server = NULL;

    TAILQ_FOREACH(seed, &driver->m_seeds, m_next) {
        mongo_seed_connect(seed);
    }

    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        mongo_server_connect(server);
    }

    mongo_driver_update_state(driver);

    return 0;
}

void mongo_driver_update_state(mongo_driver_t driver) {
    if (driver->m_state == mongo_driver_state_connecting) {
        if (driver->m_connecting_seed_count == 0 && driver->m_connecting_server_count == 0) {
            if (driver->m_master_server == NULL) {
                driver->m_state = mongo_driver_state_error;
                CPE_ERROR(
                    driver->m_em, "%s: update state: no any seed and server left, can`t find mast server!",
                    mongo_driver_name(driver));
            }
            else {
                driver->m_state = mongo_driver_state_connected;
                CPE_INFO(
                    driver->m_em, "%s: update state: connect success, master server: %s:%d!",
                    mongo_driver_name(driver), driver->m_master_server->m_host, driver->m_master_server->m_port);
            }
        }
        else {
            if (driver->m_debug) {
                driver->m_state = mongo_driver_state_error;
                CPE_ERROR(
                    driver->m_em, "%s: update_state: %d seed %d server still processing, keep connectiong!",
                    mongo_driver_name(driver), driver->m_connecting_seed_count, driver->m_connecting_server_count);
            }
        }
    }
}

int mongo_driver_enable(mongo_driver_t driver) {
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
        break;
    case mongo_driver_state_error:
        break;
    }

    return mongo_driver_connect_i(driver);
}
