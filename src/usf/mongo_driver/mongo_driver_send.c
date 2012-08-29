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

#define MONGO_BUF_APPEND_BIN(__data, __len)                 \
    assert((sizeof(buf) - writepos) >= (__len));             \
    memcpy(buf + writepos, (__data), (__len));               \
    writepos += (__len);

#define MONGO_BUF_APPEND_32(__data)                         \
    assert((sizeof(buf) - writepos) >= 4);                  \
    CPE_COPY_HTON32(buf + writepos, &(__data));             \
    writepos += 4;

int mongo_driver_send_query(mongo_driver_t driver, net_ep_t ep, mongo_pkg_t pkg) {
    char buf[128];
    size_t writepos = 0;
    size_t ns_len = strlen(pkg->m_ns) + 1;
    uint32_t doc_len = mongo_pkg_size(pkg);
    uint32_t reserve = 0;
    int32_t num_to_return = -1;

    pkg->m_pro_head.m_len =
        16 /* header */
        + sizeof(int32_t) /*  options */
        + ns_len /* ns */
        + sizeof(int32_t) /* skip */
        + sizeof(int32_t) /* return */
        + doc_len /*document*/
        + 0 /*fields*/
        ;

    MONGO_BUF_APPEND_32(pkg->m_pro_head.m_len);
    MONGO_BUF_APPEND_32(pkg->m_pro_head.m_id);
    MONGO_BUF_APPEND_32(pkg->m_pro_head.m_response_to);
    MONGO_BUF_APPEND_32(pkg->m_pro_head.m_op);

    MONGO_BUF_APPEND_32(reserve); /*options*/

    MONGO_BUF_APPEND_BIN(pkg->m_ns, ns_len); /*ns*/

    MONGO_BUF_APPEND_32(reserve); /*skip*/
    MONGO_BUF_APPEND_32(num_to_return); /*return*/
    
    if (net_ep_send(ep, buf, writepos) != 0
        || net_ep_send(ep, mongo_pkg_data(pkg), mongo_pkg_size(pkg)) != 0)
    {
        CPE_ERROR(driver->m_em, "%s: send: write head to net fail!", mongo_driver_name(driver));
        return -1;
    }

    return 0;
}

int mongo_driver_send_internal(mongo_driver_t driver, net_ep_t ep, mongo_pkg_t pkg) {
    switch(pkg->m_pro_head.m_op) {
    case mongo_db_op_query:
        return mongo_driver_send_query(driver, ep, pkg);
    default:
        CPE_ERROR(driver->m_em, "%s: send: unknown op %d!", mongo_driver_name(driver), pkg->m_pro_head.m_op);
        return -1;
    }
}

int mongo_driver_send(dp_req_t req, void * ctx, error_monitor_t em) {
    mongo_driver_t driver = ctx;
    mongo_pkg_t pkg;
    net_ep_t ep;

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

    ep = net_connector_ep(driver->m_master_server->m_connector);
    if (ep == NULL) {
        CPE_ERROR(driver->m_em, "%s: send: ep is NULL!", mongo_driver_name(driver));
        return -1;
    }

    return mongo_driver_send_internal(driver, ep, pkg);
}
