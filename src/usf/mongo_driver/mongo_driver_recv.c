#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_endpoint.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_internal_ops.h"

int mongo_driver_recv_internal(mongo_driver_t driver, net_ep_t ep, mongo_pkg_t pkg) {
    static size_t total_head_len = sizeof(struct mongo_pro_header) + sizeof(struct mongo_pro_reply_fields);
    char * buf;
    size_t buf_size;
    struct mongo_pro_header * head;
    struct mongo_pro_reply_fields * reply_fileds;

    buf_size = net_ep_size(ep);
    if (buf_size <= 0) return 0;

    if (buf_size < total_head_len) {
        if (driver->m_debug >= 3) {
            CPE_INFO(
                driver->m_em, "%s: ep %d: on read: not enouth data, head-size=%d, but only %d!",
                mongo_driver_name(driver), (int)net_ep_id(ep), (int)total_head_len, (int)buf_size);
        }
        return 0;
    }

    buf = net_ep_peek(ep, NULL, buf_size);
    if (buf == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: ep %d: peek data fail, size=%d!",
            mongo_driver_name(driver), (int)net_ep_id(ep), (int)buf_size);
        return -1;
    }

    mongo_pkg_init(pkg);

    head = (struct mongo_pro_header *)buf;
    reply_fileds = (struct mongo_pro_reply_fields *)(head + 1);

    CPE_COPY_NTOH32(&pkg->m_pro_head.m_len, &head->m_len);
    CPE_COPY_NTOH32(&pkg->m_pro_head.m_id, &head->m_id);
    CPE_COPY_NTOH32(&pkg->m_pro_head.m_response_to, &head->m_response_to);
    CPE_COPY_NTOH32(&pkg->m_pro_head.m_op, &head->m_op);

    CPE_COPY_NTOH32(&pkg->m_pro_replay_fields.m_flag, &reply_fileds->m_flag);
    CPE_COPY_NTOH32(&pkg->m_pro_replay_fields.m_cursor_id, &reply_fileds->m_cursor_id);
    CPE_COPY_NTOH32(&pkg->m_pro_replay_fields.m_start, &reply_fileds->m_start);
    CPE_COPY_NTOH32(&pkg->m_pro_replay_fields.m_num, &reply_fileds->m_num);

    if (pkg->m_pro_head.m_len < total_head_len) {
        CPE_ERROR(
            driver->m_em, "%s: ep %d: data len too small, size=%u, total_head=%d!",
            mongo_driver_name(driver), (int)net_ep_id(ep), pkg->m_pro_head.m_len, (int)total_head_len);
        return -1;
    }

    if (pkg->m_pro_head.m_len > mongo_pkg_capacity(pkg)) {
        CPE_ERROR(
            driver->m_em, "%s: ep %d: data len overflow, size=%u, max-data-len=%d!",
            mongo_driver_name(driver), (int)net_ep_id(ep), pkg->m_pro_head.m_len, (int)mongo_pkg_capacity(pkg));
        return -1;
    }

    if (pkg->m_pro_head.m_len > buf_size) {
        if (driver->m_debug >= 3) {
            CPE_INFO(
                driver->m_em, "%s: ep %d: on read: not enouth data, data-size=%u, but only %d!",
                mongo_driver_name(driver), (int)net_ep_id(ep), pkg->m_pro_head.m_len, (int)buf_size);
        }
        return 0;
    }

    memcpy(mongo_pkg_data(pkg), reply_fileds + 1, pkg->m_pro_head.m_len - total_head_len);
    mongo_pkg_set_size(pkg, pkg->m_pro_head.m_len - total_head_len);
    net_ep_erase(ep, pkg->m_pro_head.m_len);

    return 1;
}
