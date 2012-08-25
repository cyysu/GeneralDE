#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
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

    driver->m_req_max_size = 4 * 1024;
    driver->m_req_buf = NULL;

    if (cpe_hash_table_init(
            &driver->m_source_infos,
            alloc,
            (cpe_hash_fun_t) mongo_source_info_hash,
            (cpe_hash_cmp_t) mongo_source_info_eq,
            CPE_HASH_OBJ2ENTRY(mongo_source_info, m_hh),
            -1) != 0)
    {
        nm_node_free(driver_node);
        return NULL;
    }
 
    driver->m_dump_buffer_capacity = 4 * 1024;

    TAILQ_INIT(&driver->m_seeds);
    TAILQ_INIT(&driver->m_servers);

    mem_buffer_init(&driver->m_dump_buffer, driver->m_alloc);

    nm_node_set_type(driver_node, &s_nm_node_type_mongo_driver);

    return driver;
} 

static void mongo_driver_clear(nm_node_t node) {
    mongo_driver_t driver;

    driver = (mongo_driver_t)nm_node_data(node);

    if (driver->m_req_buf) {
        mongo_pkg_free(driver->m_req_buf);
        driver->m_req_buf = NULL;
    }

    mongo_source_info_free_all(driver);
    cpe_hash_table_fini(&driver->m_source_infos);

    mem_buffer_clear(&driver->m_dump_buffer);

    while(!TAILQ_EMPTY(&driver->m_servers)) {
        struct mongo_host_port * host = TAILQ_FIRST(&driver->m_servers);
        TAILQ_REMOVE(&driver->m_servers, host, m_next);
        mem_free(driver->m_alloc, host);
    }

    while(!TAILQ_EMPTY(&driver->m_seeds)) {
        struct mongo_host_port * host = TAILQ_FIRST(&driver->m_seeds);
        TAILQ_REMOVE(&driver->m_seeds, host, m_next);
        mem_free(driver->m_alloc, host);
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

int mongo_driver_add_seed(mongo_driver_t driver, const char * host, int port) {
    struct mongo_host_port * host_port;

    TAILQ_FOREACH(host_port, &driver->m_seeds, m_next) {
        if (strcmp(host_port->m_host, host) == 0 && host_port->m_port == port) return -1;
    }

    host_port = mem_alloc(driver->m_alloc, sizeof(struct mongo_host_port));
    if (host_port == NULL) return -1;

    strncpy(host_port->m_host, host, sizeof(host_port->m_host));
    host_port->m_port = port;

    TAILQ_INSERT_TAIL(&driver->m_seeds, host_port, m_next);

    return 0;
}

int mongo_driver_add_server(mongo_driver_t driver, const char * host, int port) {
    struct mongo_host_port * host_port;

    TAILQ_FOREACH(host_port, &driver->m_servers, m_next) {
        if (strcmp(host_port->m_host, host) == 0 && host_port->m_port == port) return -1;
    }

    host_port = mem_alloc(driver->m_alloc, sizeof(struct mongo_host_port));
    if (host_port == NULL) return -1;

    strncpy(host_port->m_host, host, sizeof(host_port->m_host));
    host_port->m_port = port;

    TAILQ_INSERT_TAIL(&driver->m_servers, host_port, m_next);

    return 0;
}

int mongo_driver_enable(mongo_driver_t driver) {
    return 0;
}

cpe_hash_string_t
mongo_driver_name_hs(mongo_driver_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}
