#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_endpoint.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "mongo_internal_ops.h"

int mongo_agent_send_request(mongo_agent_t agent, mongo_request_t request, logic_require_t require) {
    //if (agent->m_start != 

    if (require) {
        if (mongo_agent_save_require_id(agent, logic_require_id(require)) != 0) {
            CPE_INFO(
                agent->m_em, "%s: send_request: save require id fail!",
                mongo_agent_name(agent));
        }
    }

    return 0;
}

static void mongo_agent_on_read(mongo_agent_t agent, net_ep_t ep) {
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
            if (agent->m_debug >= 3) {
                CPE_INFO(
                    agent->m_em, "%s: ep %d: on read: not enouth data, head-size=%d, but only %d!",
                    mongo_agent_name(agent), (int)net_ep_id(ep), (int)total_head_len, (int)buf_size);
            }
            break;
        }

        buf = net_ep_peek(ep, NULL, buf_size);
        if (buf == NULL) {
            CPE_ERROR(
                agent->m_em, "%s: ep %d: peek data fail, size=%d!",
                mongo_agent_name(agent), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }

        head = (struct mongo_pro_header *)buf;
        reply_fileds = (struct mongo_pro_reply_fields *)(head + 1);

        CPE_COPY_NTOH32(&data_len, &head->m_len);

        if (data_len < total_head_len || data_len > 64*1024*1024) {
            CPE_ERROR(
                agent->m_em, "%s: ep %d: data len error, size=%u!",
                mongo_agent_name(agent), (int)net_ep_id(ep), data_len);
            net_ep_close(ep);
            break;
        }

        if (data_len > buf_size) {
            if (agent->m_debug >= 3) {
                CPE_INFO(
                    agent->m_em, "%s: ep %d: on read: not enouth data, data-size=%u, but only %d!",
                    mongo_agent_name(agent), (int)net_ep_id(ep), data_len, (int)buf_size);
            }
            break;
        }
    }
}

static void mongo_agent_on_open(mongo_agent_t agent, net_ep_t ep) {
    if(agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: ep %d: on open",
            mongo_agent_name(agent), (int)net_ep_id(ep));
    }
}

static void mongo_agent_on_close(mongo_agent_t agent, net_ep_t ep, net_ep_event_t event) {
    if(agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: ep %d: on close, event=%d",
            mongo_agent_name(agent), (int)net_ep_id(ep), event);
    }

    net_ep_free(ep);
}

void mongo_agent_ep_process(net_ep_t ep, void * ctx, net_ep_event_t event) {
    mongo_agent_t agent = (mongo_agent_t)ctx;

    assert(agent);

    switch(event) {
    case net_ep_event_read:
        mongo_agent_on_read(agent, ep);
        break;
    case net_ep_event_open:
        mongo_agent_on_open(agent, ep);
        break;
    default:
        mongo_agent_on_close(agent, ep, event);
        break;
    }
}
