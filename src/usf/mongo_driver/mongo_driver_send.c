#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_connector.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_internal_ops.h"

#define MONGO_BUF_APPEND_STR(__data)                    \
    do {                                                \
        uint32_t __len = strlen(__data) + 1;            \
        assert((sizeof(buf) - writepos) >= (__len));    \
        memcpy(buf + writepos, (__data), (__len));      \
        writepos += (__len);                            \
    } while(0)

#define MONGO_BUF_APPEND_32(__data)                         \
    assert((sizeof(buf) - writepos) >= 4);                  \
    CPE_COPY_HTON32(buf + writepos, &(__data));             \
    writepos += 4;

#define MONGO_BUF_APPEND_64(__data)                         \
    assert((sizeof(buf) - writepos) >= 8);                  \
    CPE_COPY_HTON64(buf + writepos, &(__data));             \
    writepos += 8;

static uint32_t mongo_driver_calc_len(mongo_pkg_t pkg) {
    size_t ns_len = strlen(pkg->m_ns) + 1;

    uint32_t len = 
        16 /* header */
        + mongo_pkg_size(pkg) /*document*/
        + 0 /*fields*/
        ;

    switch(pkg->m_pro_head.op) {
    case mongo_db_op_query: {
        len += sizeof(int32_t) + ns_len + sizeof(int32_t) + sizeof(int32_t);
        break;
    }
    case mongo_db_op_get_more: {
        len += sizeof(int32_t) + ns_len + sizeof(int32_t) + sizeof(int64_t);

        break;
    }
    case mongo_db_op_insert: {
        len += sizeof(int32_t) + ns_len;
        break;
    }
    case mongo_db_op_replay: {
        len += sizeof(int32_t) + sizeof(int64_t) + sizeof(int32_t) + sizeof(int32_t);
        break;
    }
    case mongo_db_op_msg: {
        break;
    }
    case mongo_db_op_kill_cursors: {
        len += sizeof(int32_t) + sizeof(int32_t);
        break;
    }
    case mongo_db_op_update: {
        len += sizeof(int32_t) + ns_len + sizeof(int32_t);
        break;
    }
    case mongo_db_op_delete: {
        len += sizeof(int32_t) + ns_len + sizeof(int32_t);
        break;
    }
    }

    return len;
}

int mongo_driver_send_to_server(mongo_driver_t driver, struct mongo_server * server, mongo_pkg_t pkg) {
    char buf[128];
    size_t writepos = 0;
    uint32_t reserve = 0;
    uint32_t len = mongo_driver_calc_len(pkg);
    net_ep_t ep;

    ep = net_connector_ep(server->m_connector);
    if (ep == NULL) {
        CPE_ERROR(driver->m_em, "%s: server %s %d: ep is NULL!", mongo_driver_name(driver), server->m_host, server->m_port);
        return -1;
    }

    MONGO_BUF_APPEND_32(len); /*messageLength*/
    MONGO_BUF_APPEND_32(pkg->m_pro_head.id); /*requestID*/
    MONGO_BUF_APPEND_32(pkg->m_pro_head.response_to); /*responseTo*/
    MONGO_BUF_APPEND_32(pkg->m_pro_head.op); /*opCode*/

    switch(pkg->m_pro_head.op) {
    case mongo_db_op_query: {
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_query.flags);
        MONGO_BUF_APPEND_STR(pkg->m_ns); /*ns*/
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_query.number_to_skip);
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_query.number_to_return);
        break;
    }
    case mongo_db_op_get_more: {
        MONGO_BUF_APPEND_32(reserve); /*options*/
        MONGO_BUF_APPEND_STR(pkg->m_ns); /*ns*/
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_get_more.number_to_return);
        MONGO_BUF_APPEND_64(pkg->m_pro_data.m_get_more.cursor_id);
        break;
    }
    case mongo_db_op_insert: {
        MONGO_BUF_APPEND_32(reserve); /*options*/
        MONGO_BUF_APPEND_STR(pkg->m_ns); /*ns*/
        break;
    }
    case mongo_db_op_replay: {
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_reply.response_flag);
        MONGO_BUF_APPEND_64(pkg->m_pro_data.m_reply.cursor_id);
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_reply.starting_from);
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_reply.number_retuned);
        break;
    }
    case mongo_db_op_msg: {
        break;
    }
    case mongo_db_op_kill_cursors: {
        MONGO_BUF_APPEND_32(reserve); /*options*/
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_kill_cursor.cursor_count);
        break;
    }
    case mongo_db_op_update: {
        MONGO_BUF_APPEND_32(reserve); /*options*/
        MONGO_BUF_APPEND_STR(pkg->m_ns); /*ns*/
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_update.flags);
        break;
    }
    case mongo_db_op_delete: {
        MONGO_BUF_APPEND_32(reserve); /*options*/
        MONGO_BUF_APPEND_STR(pkg->m_ns);
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_delete.flags);
        break;
    }
    default:
        CPE_ERROR(driver->m_em, "%s: send: unknown op %d!", mongo_driver_name(driver), pkg->m_pro_head.op);
        return -1;
    }

    if (net_ep_send(ep, buf, writepos) != 0
        || net_ep_send(ep, mongo_pkg_data(pkg), mongo_pkg_size(pkg)) != 0)
    {
        CPE_ERROR(driver->m_em, "%s: send: write head to net fail!", mongo_driver_name(driver));
        return -1;
    }

    if (driver->m_debug >= 2) {
        struct mem_buffer buffer;
        mem_buffer_init(&buffer, driver->m_alloc);

        CPE_INFO(
            driver->m_em, "%s: server %s %d: send one pkg:\n%s",
            mongo_driver_name(driver), server->m_host, server->m_port, mongo_pkg_dump(pkg, &buffer, 1));

        mem_buffer_clear(&buffer);
    }

    return 0;
}

int mongo_driver_send(dp_req_t req, void * ctx, error_monitor_t em) {
    mongo_driver_t driver = ctx;
    mongo_pkg_t pkg;

    pkg = mongo_pkg_from_dp_req(req);
    if (pkg == NULL) {
        CPE_ERROR(
            em, "%s: send: input req type %s error!",
            mongo_driver_name(driver), dp_req_type(req));
        return -1;
    }

    if (driver->m_state != mongo_driver_state_connected) {
        CPE_ERROR(
            em, "%s: send: not connected, state=%d!",
            mongo_driver_name(driver), driver->m_state);
        return -1;
    }
 
    if (driver->m_master_server == NULL) {
        CPE_ERROR(driver->m_em, "%s: send: master server not exist!", mongo_driver_name(driver));
        return -1;
    }

    return mongo_driver_send_to_server(driver, driver->m_master_server, pkg);
}
