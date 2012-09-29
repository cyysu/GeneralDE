#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_endpoint.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_internal_ops.h"

#define MONGO_BUF_READ_32(__value)                           \
    assert((read_pos + 4) <= (buf_size));                    \
    CPE_COPY_NTOH32(&(__value), buf + read_pos);             \
    read_pos += 4

#define MONGO_BUF_READ_64(__value)                           \
    assert((read_pos + 8) <= (buf_size));                    \
    CPE_COPY_NTOH64(&(__value), buf + read_pos);             \
    read_pos += 8


enum mongo_pkg_recv_result
mongo_driver_recv_internal(mongo_driver_t driver, net_ep_t ep, mongo_pkg_t pkg) {
    static uint32_t total_head_len = sizeof(struct mongo_pro_header) + sizeof(struct mongo_pro_reply_fields);
    char * buf;
    uint32_t buf_size;
    uint32_t read_pos;

REENTER:
    buf_size = net_ep_size(ep);
    if (buf_size < total_head_len) {
        if (driver->m_debug >= 3) {
            CPE_INFO(
                driver->m_em, "%s: ep %d: on read: not enouth data, head-size=%d, but only %d!",
                mongo_driver_name(driver), (int)net_ep_id(ep), (int)total_head_len, (int)buf_size);
        }
        return mongo_pkg_recv_not_enough_data;
    }

    buf = net_ep_peek(ep, NULL, buf_size);
    if (buf == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: ep %d: peek data fail, size=%d!",
            mongo_driver_name(driver), (int)net_ep_id(ep), (int)buf_size);
        return mongo_pkg_recv_error;
    }

    mongo_pkg_init(pkg);

    read_pos = 0;

    MONGO_BUF_READ_32(pkg->m_pro_head.m_len);
    MONGO_BUF_READ_32(pkg->m_pro_head.m_id);
    MONGO_BUF_READ_32(pkg->m_pro_head.m_response_to);
    MONGO_BUF_READ_32(pkg->m_pro_head.m_op);

    if (pkg->m_pro_head.m_len > buf_size) {
        if (driver->m_debug >= 3) {
            CPE_INFO(
                driver->m_em, "%s: ep %d: on read: not enouth data, data-size=%u, but only %d!",
                mongo_driver_name(driver), (int)net_ep_id(ep), pkg->m_pro_head.m_len, (int)buf_size);
        }
        return mongo_pkg_recv_not_enough_data;
    }

    if (pkg->m_pro_head.m_len < total_head_len) {
        CPE_ERROR(
            driver->m_em, "%s: ep %d: data len too small, size=%u, total_head=%d!",
            mongo_driver_name(driver), (int)net_ep_id(ep), pkg->m_pro_head.m_len, (int)total_head_len);
        net_ep_erase(ep, pkg->m_pro_head.m_len);
        return mongo_pkg_recv_error;
    }

    if (pkg->m_pro_head.m_op != mongo_db_op_replay) {
        CPE_ERROR(
            driver->m_em, "%s: ep %d: op(%d) is not reply, skip!",
            mongo_driver_name(driver), (int)net_ep_id(ep), pkg->m_pro_head.m_op);
        net_ep_erase(ep, pkg->m_pro_head.m_len);
        goto REENTER;
    }

    MONGO_BUF_READ_32(pkg->m_pro_replay_fields.m_flag);
    MONGO_BUF_READ_64(pkg->m_pro_replay_fields.m_cursor_id);
    MONGO_BUF_READ_32(pkg->m_pro_replay_fields.m_start);
    MONGO_BUF_READ_32(pkg->m_pro_replay_fields.m_num);

    if (pkg->m_pro_head.m_len > total_head_len) {
        uint32_t doc_size = pkg->m_pro_head.m_len - total_head_len;
        if (doc_size < MONGO_EMPTY_DOCUMENT_SIZE) {
            CPE_ERROR(
                driver->m_em, "%s: ep %d: doc len is too small for empty doc, doc-len=%d!",
                mongo_driver_name(driver), (int)net_ep_id(ep), pkg->m_pro_head.m_len - total_head_len);
            net_ep_erase(ep, pkg->m_pro_head.m_len);
            return mongo_pkg_recv_error;
        }

        memcpy(mongo_pkg_data(pkg), buf + read_pos, doc_size);
        read_pos += doc_size;
        mongo_pkg_set_size(pkg, doc_size);
    }

    net_ep_erase(ep, pkg->m_pro_head.m_len);

    return mongo_pkg_recv_ok;
}
