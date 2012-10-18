#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_connector.h"
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "mongo_internal_ops.h"

int mongo_driver_add_seed(mongo_driver_t driver, const char * host, int port) {
    struct mongo_seed * seed;

    TAILQ_FOREACH(seed, &driver->m_seeds, m_next) {
        if (strcmp(seed->m_host, host) == 0 && seed->m_port == port) return -1;
    }

    seed = mem_alloc(driver->m_alloc, sizeof(struct mongo_seed));
    if (seed == NULL) return -1;

    seed->m_driver = driver;
    strncpy(seed->m_host, host, sizeof(seed->m_host));
    seed->m_port = port;
    seed->m_connector = NULL;

    TAILQ_INSERT_TAIL(&driver->m_seeds, seed, m_next);
    ++driver->m_seed_count;

    return 0;
}

void mongo_seed_free(struct mongo_seed * seed) {
    mongo_driver_t driver = seed->m_driver;

    TAILQ_REMOVE(&driver->m_seeds, seed, m_next);

    if (seed->m_connector) net_connector_free(seed->m_connector);

    mem_free(driver->m_alloc, seed);
}

void mongo_seed_free_all(mongo_driver_t driver) {
    while(!TAILQ_EMPTY(&driver->m_seeds)) {
        mongo_seed_free(TAILQ_FIRST(&driver->m_seeds));
    }
}


static void mongo_seed_free_chanel_buf(net_chanel_t chanel, void * ctx) {
    mongo_driver_t driver = (mongo_driver_t)ctx;
    assert(driver);
    mem_free(driver->m_alloc, net_chanel_queue_buf(chanel));
}

static void mongo_seed_connector_state_monitor(net_connector_t connector, void * ctx) {
    
}

static void mongo_seed_recv_on_connecting(net_ep_t ep, void * ctx, net_ep_event_t event) {
}

int mongo_seed_ep_init(mongo_driver_t driver, net_ep_t ep) {
    void * buf_r = NULL;
    void * buf_w = NULL;
    net_chanel_t chanel_r = NULL;
    net_chanel_t chanel_w = NULL;
    size_t read_chanel_size = 4 * 1024;
    size_t write_chanel_size = 1024;

    assert(driver);

    buf_r = mem_alloc(driver->m_alloc, read_chanel_size);
    buf_w = mem_alloc(driver->m_alloc, write_chanel_size);
    if (buf_r == NULL || buf_w == NULL) goto EP_INIT_ERROR;

    chanel_r = net_chanel_queue_create(net_ep_mgr(ep), buf_r, read_chanel_size);
    if (chanel_r == NULL) goto EP_INIT_ERROR;
    net_chanel_queue_set_close(chanel_r, mongo_seed_free_chanel_buf, driver);
    buf_r = NULL;

    chanel_w = net_chanel_queue_create(net_ep_mgr(ep), buf_w, write_chanel_size);
    if (chanel_w == NULL) goto EP_INIT_ERROR;
    net_chanel_queue_set_close(chanel_w, mongo_seed_free_chanel_buf, driver);
    buf_w = NULL;

    net_ep_set_chanel_r(ep, chanel_r);
    chanel_r = NULL;

    net_ep_set_chanel_w(ep, chanel_w);
    chanel_w = NULL;

    net_ep_set_processor(ep, mongo_seed_recv_on_connecting, driver);

    if(driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: ep %d: init success!",
            mongo_driver_name(driver), (int)net_ep_id(ep));
    }

    return 0;
EP_INIT_ERROR:
    if (buf_r) mem_free(driver->m_alloc, buf_r);
    if (buf_w) mem_free(driver->m_alloc, buf_w);
    if (chanel_r) net_chanel_free(chanel_r);
    if (chanel_w) net_chanel_free(chanel_w);
    net_ep_close(ep);

    CPE_ERROR(
        driver->m_em, "%s: ep %d: init fail!",
        mongo_driver_name(driver), (int)net_ep_id(ep));

    return -1;
}

int mongo_seed_connect(struct mongo_seed * seed) {
    mongo_driver_t driver = seed->m_driver;

    if (seed->m_connector) net_connector_free(seed->m_connector);

    seed->m_connector =
        net_connector_create_with_ep(gd_app_net_mgr(driver->m_app), seed->m_host, seed->m_host, seed->m_port);
    if (seed->m_connector == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: seed %s.%d: create net connector fail!",
            mongo_driver_name(driver), seed->m_host, seed->m_port);
        goto CONNECT_ERROR;
    }

    if (mongo_seed_ep_init(driver, net_connector_ep(seed->m_connector)) != 0) {
        goto CONNECT_ERROR;
    }

    if (net_connector_add_monitor(seed->m_connector, mongo_seed_connector_state_monitor, seed) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: seed %s.%d: connector add monitor fail!",
            mongo_driver_name(driver), seed->m_host, seed->m_port);
        goto CONNECT_ERROR;
    }

    if (net_connector_enable(seed->m_connector) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: seed %s.%d: enable connector fail!",
            mongo_driver_name(driver), seed->m_host, seed->m_port);
        goto CONNECT_ERROR;
    }

    if (driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: seed %s.%d: connecting!",
            mongo_driver_name(driver), seed->m_host, seed->m_port);
    }

    ++driver->m_connecting_seed_count;
    return 0;

CONNECT_ERROR:
    if (seed->m_connector) {
        net_connector_free(seed->m_connector);
        seed->m_connector = NULL;
    }

    return -1;
}
