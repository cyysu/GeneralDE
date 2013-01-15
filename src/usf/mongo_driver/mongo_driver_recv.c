#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_endpoint.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_internal_ops.h"

#define MONGO_BUF_READ_32(__value)                                      \
    if (read_pos + 4 > buf_size) return mongo_pkg_recv_not_enough_data; \
    CPE_COPY_NTOH32(&(__value), buf + read_pos);                        \
    read_pos += 4

#define MONGO_BUF_READ_64(__value)                                      \
    if (read_pos + 8 > buf_size) return mongo_pkg_recv_not_enough_data; \
    CPE_COPY_NTOH64(&(__value), buf + read_pos);                        \
    read_pos += 8

#define MONGO_BUF_READ_STR(__value)                                     \
    do {                                                                \
        uint32_t __len = strlen(buf + read_pos);                        \
        if (read_pos + __len + 1 > buf_size) return mongo_pkg_recv_not_enough_data; \
        if (__len + 1 > sizeof(__value)) __len = sizeof(__value) - 1;   \
        memcpy(__value, buf + read_pos, __len);                         \
        __value[__len] = 0;                                             \
        read_pos += __len + 1;                                          \
    } while(0)

enum mongo_pkg_recv_result
mongo_driver_recv_internal(mongo_driver_t driver, net_ep_t ep, mongo_pkg_t pkg) {
    char * buf;
    uint32_t buf_size;
    uint32_t read_pos;
    uint32_t total_len;
    uint32_t reserve;
    char ns_buf[65];

REENTER:
    buf_size = net_ep_size(ep);

    buf = net_ep_peek(ep, NULL, buf_size);
    if (buf == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: ep %d: peek data fail, size=%d!",
            mongo_driver_name(driver), (int)net_ep_id(ep), (int)buf_size);
        return mongo_pkg_recv_error;
    }

    mongo_pkg_init(pkg);

    read_pos = 0;

    MONGO_BUF_READ_32(total_len);
    MONGO_BUF_READ_32(pkg->m_pro_head.id);
    MONGO_BUF_READ_32(pkg->m_pro_head.response_to);
    MONGO_BUF_READ_32(pkg->m_pro_head.op);

    if (total_len > buf_size) {
        if (driver->m_debug >= 3) {
            CPE_INFO(
                driver->m_em, "%s: ep %d: on read: not enouth data, data-size=%u, but only %d!",
                mongo_driver_name(driver), (int)net_ep_id(ep), total_len, (int)buf_size);
        }
        return mongo_pkg_recv_not_enough_data;
    }

    switch(pkg->m_pro_head.op) {
    case mongo_db_op_query: {
        MONGO_BUF_READ_32(pkg->m_pro_data.m_query.flags);
        MONGO_BUF_READ_STR(ns_buf); /*ns*/
        mongo_pkg_set_ns(pkg, ns_buf);
        MONGO_BUF_READ_32(pkg->m_pro_data.m_query.number_to_skip);
        MONGO_BUF_READ_32(pkg->m_pro_data.m_query.number_to_return);
        break;
    }
    case mongo_db_op_get_more: {
        MONGO_BUF_READ_32(reserve); /*options*/
        MONGO_BUF_READ_STR(ns_buf); /*ns*/
        mongo_pkg_set_ns(pkg, ns_buf);
        MONGO_BUF_READ_32(pkg->m_pro_data.m_get_more.number_to_return);
        MONGO_BUF_READ_64(pkg->m_pro_data.m_get_more.cursor_id);
        break;
    }
    case mongo_db_op_insert: {
        MONGO_BUF_READ_32(reserve); /*options*/
        MONGO_BUF_READ_STR(ns_buf); /*ns*/
        mongo_pkg_set_ns(pkg, ns_buf);
        break;
    }
    case mongo_db_op_replay: {
        MONGO_BUF_READ_32(pkg->m_pro_data.m_reply.response_flag);
        MONGO_BUF_READ_64(pkg->m_pro_data.m_reply.cursor_id);
        MONGO_BUF_READ_32(pkg->m_pro_data.m_reply.starting_from);
        MONGO_BUF_READ_32(pkg->m_pro_data.m_reply.number_retuned);
        break;
    }
    case mongo_db_op_msg: {
        break;
    }
    case mongo_db_op_kill_cursors: {
        MONGO_BUF_READ_32(reserve); /*options*/
        MONGO_BUF_READ_32(pkg->m_pro_data.m_kill_cursor.cursor_count);
        break;
    }
    case mongo_db_op_update: {
        MONGO_BUF_READ_32(reserve); /*options*/
        MONGO_BUF_READ_STR(ns_buf); /*ns*/
        mongo_pkg_set_ns(pkg, ns_buf);
        MONGO_BUF_READ_32(pkg->m_pro_data.m_update.flags);
        break;
    }
    case mongo_db_op_delete: {
        MONGO_BUF_READ_32(reserve); /*options*/
        MONGO_BUF_READ_STR(ns_buf); /*ns*/
        mongo_pkg_set_ns(pkg, ns_buf);
        MONGO_BUF_READ_32(pkg->m_pro_data.m_delete.flags);
        break;
    }
    default:
        CPE_ERROR(
            driver->m_em, "%s: ep %d: op(%d) is not reply, skip!",
            mongo_driver_name(driver), (int)net_ep_id(ep), pkg->m_pro_head.op);
        net_ep_erase(ep, total_len);
        goto REENTER;
    }

    assert(total_len >= read_pos);

    if (total_len > read_pos) {
        uint32_t doc_size = total_len - read_pos;
        if (doc_size < MONGO_EMPTY_DOCUMENT_SIZE) {
            CPE_ERROR(
                driver->m_em, "%s: ep %d: doc len is too small for empty doc, doc-len=%d!",
                mongo_driver_name(driver), (int)net_ep_id(ep), total_len - read_pos);
            net_ep_erase(ep, total_len);
            return mongo_pkg_recv_error;
        }

        memcpy(mongo_pkg_data(pkg), buf + read_pos, doc_size);
        read_pos += doc_size;
        mongo_pkg_set_size(pkg, doc_size);
        mongo_pkg_doc_count_update(pkg);
    }

    net_ep_erase(ep, total_len);

    return mongo_pkg_recv_ok;
}
