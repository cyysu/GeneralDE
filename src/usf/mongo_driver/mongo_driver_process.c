#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_endpoint.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "mongo_internal_ops.h"

static void mongo_driver_on_read(mongo_driver_t driver, net_ep_t ep) {
    static size_t total_head_len = sizeof(struct mongo_pro_header) + sizeof(struct mongo_pro_reply_fields);

    while(1) {
        char * buf;
        size_t buf_size;
        struct mongo_pro_header * head;
        struct mongo_pro_reply_fields * reply_fileds;
        uint32_t data_len;

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

        CPE_COPY_NTOH32(&data_len, &head->m_len);

        if (data_len < total_head_len || data_len > 64*1024*1024) {
            CPE_ERROR(
                driver->m_em, "%s: ep %d: data len error, size=%u!",
                mongo_driver_name(driver), (int)net_ep_id(ep), data_len);
            net_ep_close(ep);
            break;
        }

        if (data_len > buf_size) {
            if (driver->m_debug >= 3) {
                CPE_INFO(
                    driver->m_em, "%s: ep %d: on read: not enouth data, data-size=%u, but only %d!",
                    mongo_driver_name(driver), (int)net_ep_id(ep), data_len, (int)buf_size);
            }
            break;
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

void mongo_driver_ep_process(net_ep_t ep, void * ctx, net_ep_event_t event) {
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
