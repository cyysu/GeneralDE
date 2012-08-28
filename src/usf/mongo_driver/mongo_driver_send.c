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

int mongo_driver_send_internal(mongo_driver_t driver, net_ep_t ep, mongo_pkg_t pkg) {
    struct mongo_pro_header head;

    pkg->m_pro_head.m_len = mongo_pkg_size(pkg) + sizeof(head);

    CPE_COPY_HTON32(&head.m_len, &pkg->m_pro_head.m_len);
    CPE_COPY_HTON32(&head.m_id, &pkg->m_pro_head.m_id);
    CPE_COPY_HTON32(&head.m_response_to, &pkg->m_pro_head.m_response_to);
    CPE_COPY_HTON32(&head.m_op, &pkg->m_pro_head.m_op);

    if (net_ep_send(ep, &head, sizeof(head)) != 0) {
        CPE_ERROR(driver->m_em, "%s: send: write to net fail!", mongo_driver_name(driver));
        goto ERROR;
    }

    if (net_ep_send(ep, &head, sizeof(head)) != 0) {
        CPE_ERROR(driver->m_em, "%s: send: write to net fail!", mongo_driver_name(driver));
        goto ERROR;
    }
    
    return 0;

ERROR:
    return -1;
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
