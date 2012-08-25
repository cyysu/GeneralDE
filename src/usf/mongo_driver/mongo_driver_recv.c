#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_internal_ops.h"

static void mongo_driver_on_read(mongo_driver_t driver, net_ep_t ep) {
    static size_t total_head_len = sizeof(struct mongo_pro_header) + sizeof(struct mongo_pro_reply_fields);
    mongo_pkg_t req_buf;

    if(driver->m_debug >= 2) {
        CPE_INFO(
            driver->m_em, "%s: ep %d: on read",
            mongo_driver_name(driver), (int)net_ep_id(ep));
    }

    req_buf = mongo_driver_req_buf(driver);
    if (req_buf == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: ep %d: get req buf fail!",
            mongo_driver_name(driver), (int)net_ep_id(ep));
        net_ep_close(ep);
        return;
    }

    while(1) {
        char * buf;
        size_t buf_size;
        struct mongo_pro_header * head;
        struct mongo_pro_reply_fields * reply_fileds;

        buf_size = net_ep_size(ep);
        if (buf_size <= 0) break;

        if (buf_size < total_head_len) {
            if (driver->m_debug >= 3) {
                CPE_INFO(
                    driver->m_em, "%s: ep %d: on read: not enouth data, head-size=%d, but only %d!",
                    mongo_driver_name(driver), (int)net_ep_id(ep), (int)total_head_len, (int)buf_size);
            }
            break;
        }

        buf = net_ep_peek(ep, NULL, buf_size);
        if (buf == NULL) {
            CPE_ERROR(
                driver->m_em, "%s: ep %d: peek data fail, size=%d!",
                mongo_driver_name(driver), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }

        head = (struct mongo_pro_header *)buf;
        reply_fileds = (struct mongo_pro_reply_fields *)(head + 1);

        CPE_COPY_NTOH32(&req_buf->m_pro_head.m_len, &head->m_len);
        CPE_COPY_NTOH32(&req_buf->m_pro_head.m_id, &head->m_id);
        CPE_COPY_NTOH32(&req_buf->m_pro_head.m_response_to, &head->m_response_to);
        CPE_COPY_NTOH32(&req_buf->m_pro_head.m_op, &head->m_op);

        CPE_COPY_NTOH32(&req_buf->m_pro_replay_fields.m_flag, &reply_fileds->m_flag);
        CPE_COPY_NTOH32(&req_buf->m_pro_replay_fields.m_cursor_id, &reply_fileds->m_cursor_id);
        CPE_COPY_NTOH32(&req_buf->m_pro_replay_fields.m_start, &reply_fileds->m_start);
        CPE_COPY_NTOH32(&req_buf->m_pro_replay_fields.m_num, &reply_fileds->m_num);

        if (req_buf->m_pro_head.m_len < total_head_len) {
            CPE_ERROR(
                driver->m_em, "%s: ep %d: data len too small, size=%u, total_head=%d!",
                mongo_driver_name(driver), (int)net_ep_id(ep), req_buf->m_pro_head.m_len, (int)total_head_len);
            net_ep_close(ep);
            break;
        }

        if (req_buf->m_pro_head.m_len > mongo_pkg_capacity(req_buf)) {
            CPE_ERROR(
                driver->m_em, "%s: ep %d: data len overflow, size=%u, max-data-len=%d!",
                mongo_driver_name(driver), (int)net_ep_id(ep), req_buf->m_pro_head.m_len, (int)mongo_pkg_capacity(req_buf));
            net_ep_close(ep);
            break;
        }

        if (req_buf->m_pro_head.m_len > buf_size) {
            if (driver->m_debug >= 3) {
                CPE_INFO(
                    driver->m_em, "%s: ep %d: on read: not enouth data, data-size=%u, but only %d!",
                    mongo_driver_name(driver), (int)net_ep_id(ep), req_buf->m_pro_head.m_len, (int)buf_size);
            }
            break;
        }

        memcpy(mongo_pkg_data(req_buf), reply_fileds + 1, req_buf->m_pro_head.m_len - total_head_len);
        mongo_pkg_set_size(req_buf, req_buf->m_pro_head.m_len - total_head_len);
        net_ep_erase(ep, req_buf->m_pro_head.m_len);

        if (driver->m_state == mongo_driver_state_connected) {
            struct mongo_source_info * source_info = mongo_source_info_find(driver, req_buf->m_pro_head.m_response_to);
            if (source_info == NULL) {
                CPE_ERROR(
                    driver->m_em, "%s: ep %d: on read: dispatch_to %d not exist, skip!", 
                    mongo_driver_name(driver), (int)net_ep_id(ep), req_buf->m_pro_head.m_response_to);
                continue;
            }
            else {
                if (dp_dispatch_by_string(source_info->m_incoming_dsp_to, mongo_pkg_to_dp_req(req_buf), driver->m_em) != 0) {
                    CPE_ERROR(
                        driver->m_em, "%s: ep %d: on read: dispatch_to %s fail!", 
                        mongo_driver_name(driver), (int)net_ep_id(ep), cpe_hs_data(source_info->m_incoming_dsp_to));
                    continue;
                }
            }
        }
        else {
            mongo_driver_process_connect(driver, req_buf);
        }
    }
}

static void mongo_driver_on_open(mongo_driver_t driver, net_ep_t ep) {
    if(driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: ep %d: on open",
            mongo_driver_name(driver), (int)net_ep_id(ep));
    }
}

static void mongo_driver_on_close(mongo_driver_t driver, net_ep_t ep, net_ep_event_t event) {
    if(driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: ep %d: on close, event=%d",
            mongo_driver_name(driver), (int)net_ep_id(ep), event);
    }

    net_ep_free(ep);
}

void mongo_driver_recv(net_ep_t ep, void * ctx, net_ep_event_t event) {
    mongo_driver_t driver = (mongo_driver_t)ctx;

    assert(driver);

    switch(event) {
    case net_ep_event_read:
        mongo_driver_on_read(driver, ep);
        break;
    case net_ep_event_open:
        mongo_driver_on_open(driver, ep);
        break;
    default:
        mongo_driver_on_close(driver, ep, event);
        break;
    }
}
