#include <assert.h>
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

static void center_agent_on_read(center_agent_t agent, net_ep_t ep) {
}

static void center_agent_on_open(center_agent_t agent, net_ep_t ep) {
    if(agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: ep %d: on open",
            center_agent_name(agent), (int)net_ep_id(ep));
    }

}

static void center_agent_on_close(center_agent_t agent, net_ep_t ep, net_ep_event_t event) {
    if(agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: ep %d: on close, event=%d",
            center_agent_name(agent), (int)net_ep_id(ep), event);
    }
}

static void center_agent_process(net_ep_t ep, void * ctx, net_ep_event_t event) {
    center_agent_t agent = (center_agent_t)ctx;

    assert(agent);

    switch(event) {
    case net_ep_event_read:
        center_agent_on_read(agent, ep);
        break;
    case net_ep_event_open:
        center_agent_on_open(agent, ep);
        break;
    default:
        center_agent_on_close(agent, ep, event);
        break;
    }
}

static void center_agent_free_chanel_buf(net_chanel_t chanel, void * ctx) {
    center_agent_t agent = (center_agent_t)ctx;

    assert(agent);

    mem_free(agent->m_alloc, net_chanel_queue_buf(chanel));
}

int center_agent_ep_init(center_agent_t agent, net_ep_t ep) {
    void * buf_r = NULL;
    void * buf_w = NULL;
    net_chanel_t chanel_r = NULL;
    net_chanel_t chanel_w = NULL;

    assert(agent);

    buf_r = mem_alloc(agent->m_alloc, agent->m_read_chanel_size);
    buf_w = mem_alloc(agent->m_alloc, agent->m_write_chanel_size);
    if (buf_r == NULL || buf_w == NULL) goto EP_INIT_ERROR;

    chanel_r = net_chanel_queue_create(net_ep_mgr(ep), buf_r, agent->m_read_chanel_size);
    if (chanel_r == NULL) goto EP_INIT_ERROR;
    net_chanel_queue_set_close(chanel_r, center_agent_free_chanel_buf, agent);
    buf_r = NULL;

    chanel_w = net_chanel_queue_create(net_ep_mgr(ep), buf_w, agent->m_write_chanel_size);
    if (chanel_w == NULL) goto EP_INIT_ERROR;
    net_chanel_queue_set_close(chanel_w, center_agent_free_chanel_buf, agent);
    buf_w = NULL;

    net_ep_set_chanel_r(ep, chanel_r);
    chanel_r = NULL;

    net_ep_set_chanel_w(ep, chanel_w);
    chanel_w = NULL;

    net_ep_set_processor(ep, center_agent_process, agent);

    return 0;
EP_INIT_ERROR:
    if (buf_r) mem_free(agent->m_alloc, buf_r);
    if (buf_w) mem_free(agent->m_alloc, buf_w);
    if (chanel_r) net_chanel_free(chanel_r);
    if (chanel_w) net_chanel_free(chanel_w);
    net_ep_close(ep);

    CPE_ERROR(
        agent->m_em, "%s: ep %d: init fail!",
        center_agent_name(agent), (int)net_ep_id(ep));

    return -1;
}
