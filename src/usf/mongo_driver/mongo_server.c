#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/net/net_connector.h"
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_internal_ops.h"

static int mongo_server_ep_init(struct mongo_server * server, net_ep_t ep);
static void mongo_server_connector_state_monitor(net_connector_t connector, void * ctx);

int mongo_driver_add_server(mongo_driver_t driver, const char * host, int port) {
    struct mongo_server * server;

    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        if (strcmp(server->m_host, host) == 0 && server->m_port == port) return -1;
    }

    server = mem_alloc(driver->m_alloc, sizeof(struct mongo_server));
    if (server == NULL) return -1;

    server->m_driver = driver;
    strncpy(server->m_host, host, sizeof(server->m_host));
    server->m_port = port;
    server->m_connector = NULL;
    server->m_state = mongo_server_state_init;
    server->m_max_bson_size = MONGO_DEFAULT_MAX_BSON_SIZE;

    server->m_connector =
        net_connector_create_with_ep(gd_app_net_mgr(driver->m_app), server->m_host, server->m_host, server->m_port);
    if (server->m_connector == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: create: create net connector fail!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        mem_free(driver->m_alloc, server);
        return -1;
    }

    if (mongo_server_ep_init(server, net_connector_ep(server->m_connector)) != 0) {
        net_connector_free(server->m_connector);
        mem_free(driver->m_alloc, server);
        return -1;
    }

    if (net_connector_add_monitor(server->m_connector, mongo_server_connector_state_monitor, server) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: connector add monitor fail!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        net_connector_free(server->m_connector);
        mem_free(driver->m_alloc, server);
        return -1;
    }

    TAILQ_INSERT_TAIL(&driver->m_servers, server, m_next);
    ++driver->m_server_count;

    return 0;
}

void mongo_server_free(struct mongo_server * server) {
    mongo_driver_t driver = server->m_driver;

    mongo_server_disable(server);

    if (server->m_connector) {
        net_connector_free(server->m_connector);
        server->m_connector = NULL;
    }

    TAILQ_REMOVE(&driver->m_servers, server, m_next);

    mem_free(driver->m_alloc, server);
}

void mongo_server_free_all(mongo_driver_t driver) {
    while(!TAILQ_EMPTY(&driver->m_servers)) {
        mongo_server_free(TAILQ_FIRST(&driver->m_servers));
    }
}

static void mongo_server_disable_i(struct mongo_server * server, enum mongo_server_state next_state) {
    if (server->m_state != mongo_server_state_init
        && server->m_state != mongo_server_state_connected
        && server->m_state != mongo_server_state_error)
    {
        --server->m_driver->m_connecting_server_count;
        server->m_state = next_state;
    }

    if (server->m_driver->m_master_server == server) {
        server->m_driver->m_master_server = NULL;
    }

    if (server->m_connector) net_connector_disable(server->m_connector);

    mongo_driver_update_state(server->m_driver);
}

void mongo_server_error(struct mongo_server * server) {
    mongo_server_disable_i(server, mongo_server_state_error);
}

void mongo_server_disable(struct mongo_server * server) {
    mongo_server_disable_i(server, mongo_server_state_init);
}

static void mongo_server_free_chanel_buf(net_chanel_t chanel, void * ctx) {
    mongo_driver_t driver = (mongo_driver_t)ctx;
    assert(driver);
    mem_free(driver->m_alloc, net_chanel_queue_buf(chanel));
}

static void mongo_server_check_is_master(struct mongo_server * server) {
    mongo_driver_t driver = server->m_driver;
    mongo_pkg_t pkg_buf = mongo_driver_pkg_buf(driver);
    net_ep_t ep;

    ep = net_connector_ep(server->m_connector);
    if (ep == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: check is master: ep is null!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        mongo_server_error(server);
        return;
    }

    if (pkg_buf == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: check is master: get pkg buf fail!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        mongo_server_error(server);
        return;
    }

    server->m_state = mongo_server_state_checking_is_master;

    mongo_pkg_set_ns(pkg_buf, "admin.$cmd");
    mongo_pkg_set_op(pkg_buf, mongo_db_op_query);
<<<<<<< HEAD
    if (mongo_pkg_doc_add(pkg_buf) != 0
=======
    if (mongo_pkg_doc_open(pkg_buf) != 0
>>>>>>> 5aebc81cb0ca2f0d0a569701c102fa4cf9abd362
        || mongo_pkg_append_int32(pkg_buf, "ismaster", 1) != 0
        || mongo_pkg_doc_close(pkg_buf) != 0
        || mongo_driver_send_internal(driver, ep, pkg_buf))
    {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: check is master: send cmd fail!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        mongo_server_error(server);
        return;
    }

    if (driver->m_debug) {
        if (driver->m_debug >= 2) {
            struct mem_buffer buffer;
            mem_buffer_init(&buffer, driver->m_alloc);

            CPE_INFO(
                driver->m_em, "%s: server %s %d: check is master: send cmd success\n%s",
                mongo_driver_name(driver), server->m_host, server->m_port, mongo_pkg_dump(pkg_buf, &buffer, 1));

            mem_buffer_clear(&buffer);
        }
        else {
            CPE_INFO(
                driver->m_em, "%s: server %s %d: check is master: send cmd success!",
                mongo_driver_name(driver), server->m_host, server->m_port);
        }
    }
}

static void mongo_server_on_check_is_master(struct mongo_server * server, mongo_pkg_t pkg) {
    mongo_driver_t driver = server->m_driver;
    bson_iterator it;

<<<<<<< HEAD
    mongo_pkg_it(pkg, &it);

    if(mongo_pkg_find(pkg, &it, "maxBsonObjectSize")) {
=======
    mongo_pkg_it(&it, pkg, 0);

    if(mongo_pkg_find(&it, pkg, 0, "maxBsonObjectSize")) {
>>>>>>> 5aebc81cb0ca2f0d0a569701c102fa4cf9abd362
        server->m_max_bson_size = bson_iterator_int(&it);
    }
    else {
        server->m_max_bson_size = MONGO_DEFAULT_MAX_BSON_SIZE;
    }

<<<<<<< HEAD
    if(mongo_pkg_find(pkg, &it, "ismaster")) {
=======
    if(mongo_pkg_find(&it, pkg, 0, "ismaster")) {
>>>>>>> 5aebc81cb0ca2f0d0a569701c102fa4cf9abd362
        if (bson_iterator_bool( &it ) ) {
            if (driver->m_master_server) {
                CPE_INFO(
                    driver->m_em, "%s: server %s %d: is master, replace old master %s %d!",
                    mongo_driver_name(driver), server->m_host, server->m_port, driver->m_master_server->m_host, driver->m_master_server->m_port);
            }
            else {
                if (driver->m_debug) {
                    CPE_INFO(
                        driver->m_em, "%s: server %s %d: is master, set to driver!",
                        mongo_driver_name(driver), server->m_host, server->m_port);
                }
            }
            server->m_state = mongo_server_state_connected;
            mongo_driver_update_state(driver);
        }
        else {
            mongo_server_error(server);
        }
    }
    else {
        mongo_server_error(server);
    }
}

static void mongo_server_connector_state_monitor(net_connector_t connector, void * ctx) {
    struct mongo_server * server = ctx;
    mongo_driver_t driver = server->m_driver;

    switch(net_connector_state(connector)) {
    case net_connector_state_connecting:
        return;
    case net_connector_state_connected:
        break;
    case net_connector_state_error:
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: connect state changed to error!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        mongo_server_error(server);
        return;
    case net_connector_state_disable:
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: connect state changed to disable!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        mongo_server_error(server);
        return;
    case net_connector_state_idle:
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: connect state changed to idle!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        mongo_server_error(server);
        return;
    }

    if (driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: connect success!",
            mongo_driver_name(driver), server->m_host, server->m_port);
    }

    mongo_server_check_is_master(server);
}

static void mongo_server_on_read(struct mongo_server * server, net_ep_t ep) {
    mongo_driver_t driver = server->m_driver;
    mongo_pkg_t req_buf;

    if(driver->m_debug >= 2) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: on read",
            mongo_driver_name(driver), server->m_host, server->m_port);
    }

    req_buf = mongo_driver_pkg_buf(driver);
    if (req_buf == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: on read: get pkg buf fail!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        mongo_server_error(server);
        return;
    }

    while(1) {
        enum mongo_pkg_recv_result r = mongo_driver_recv_internal(driver, ep, req_buf);

        if (r == mongo_pkg_recv_not_enough_data) {
            break;
        }
        else if (r == mongo_pkg_recv_error) {
            mongo_server_error(server);
            return;
        }

        if (driver->m_debug >= 2) {
            struct mem_buffer buffer;
            mem_buffer_init(&buffer, driver->m_alloc);

            CPE_INFO(
                driver->m_em, "%s: server %s %d: receive one pkg:\n%s",
                mongo_driver_name(driver), server->m_host, server->m_port, mongo_pkg_dump(req_buf, &buffer, 1));

            mem_buffer_clear(&buffer);
        }

        switch(server->m_state) {
        case mongo_server_state_init:
        case mongo_server_state_connecting:
        case mongo_server_state_error:
            CPE_ERROR(
                driver->m_em, "%s: server %s %d: on read: receive data in error state, state=%d!",
                mongo_driver_name(driver), server->m_host, server->m_port, server->m_state);
            break;
        case mongo_server_state_checking_is_master:
            mongo_server_on_check_is_master(server, req_buf);
            break;
        case mongo_server_state_connected:
            if (dp_dispatch_by_string(driver->m_incoming_send_to, mongo_pkg_to_dp_req(req_buf), driver->m_em) != 0) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s %d: on read: dispatch to %s fail!",
                    mongo_driver_name(driver), server->m_host, server->m_port, cpe_hs_data(driver->m_incoming_send_to));
            }
            break;
        }
    }
}

static void mongo_server_on_open(struct mongo_server * server, net_ep_t ep) {
    mongo_driver_t driver = server->m_driver;

    if(driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: on open!",
            mongo_driver_name(driver), server->m_host, server->m_port);
    }
}

static void mongo_server_on_close(struct mongo_server * server, net_ep_t ep, net_ep_event_t event) {
    mongo_driver_t driver = server->m_driver;

    if(driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: ep %d: on close, event=%d!",
            mongo_driver_name(driver), server->m_host, server->m_port, (int)net_ep_id(ep), event);
    }

    if (server == driver->m_master_server) {
        mongo_driver_connect_i(driver);
    }
}

static void mongo_server_recv_on_connecting(net_ep_t ep, void * ctx, net_ep_event_t event) {
    struct mongo_server * server = (struct mongo_server *)ctx;

    assert(server);

    switch(event) {
    case net_ep_event_read:
        mongo_server_on_read(server, ep);
        break;
    case net_ep_event_open:
        mongo_server_on_open(server, ep);
        break;
    default:
        mongo_server_on_close(server, ep, event);
        break;
    }
}

static int mongo_server_ep_init(struct mongo_server * server, net_ep_t ep) {
    void * buf_r = NULL;
    void * buf_w = NULL;
    net_chanel_t chanel_r = NULL;
    net_chanel_t chanel_w = NULL;
    mongo_driver_t driver = server->m_driver;

    assert(driver);

    buf_r = mem_alloc(driver->m_alloc, driver->m_server_read_chanel_size);
    buf_w = mem_alloc(driver->m_alloc, driver->m_server_write_chanel_size);
    if (buf_r == NULL || buf_w == NULL) goto ERROR;

    chanel_r = net_chanel_queue_create(net_ep_mgr(ep), buf_r, driver->m_server_read_chanel_size);
    if (chanel_r == NULL) goto ERROR;
    net_chanel_queue_set_close(chanel_r, mongo_server_free_chanel_buf, driver);
    buf_r = NULL;

    chanel_w = net_chanel_queue_create(net_ep_mgr(ep), buf_w, driver->m_server_write_chanel_size);
    if (chanel_w == NULL) goto ERROR;
    net_chanel_queue_set_close(chanel_w, mongo_server_free_chanel_buf, driver);
    buf_w = NULL;

    net_ep_set_chanel_r(ep, chanel_r);
    chanel_r = NULL;

    net_ep_set_chanel_w(ep, chanel_w);
    chanel_w = NULL;

    net_ep_set_processor(ep, mongo_server_recv_on_connecting, server);

    if(driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: ep %d: init success!",
            mongo_driver_name(driver), (int)net_ep_id(ep));
    }

    return 0;
ERROR:
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

int mongo_server_connect(struct mongo_server * server) {
    mongo_driver_t driver = server->m_driver;

    if (net_connector_enable(server->m_connector) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: enable connector fail!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        goto ERROR;
    }

    if (driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: start connect!",
            mongo_driver_name(driver), server->m_host, server->m_port);
    }

    server->m_state = mongo_server_state_connecting;
    ++driver->m_connecting_server_count;
    return 0;

ERROR:
    net_connector_disable(server->m_connector);
    return -1;
}
