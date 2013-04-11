#include <assert.h>
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

static center_agent_center_op_t g_center_agent_center_ops[] = {
    NULL
    , NULL, NULL
    , NULL, NULL
};


void * center_agent_get_incoming_pkg_buff(center_agent_t agent, size_t capacity) {
    if (mem_buffer_size(&agent->m_incoming_pkg_buf) < capacity) {
        if (mem_buffer_set_size(&agent->m_incoming_pkg_buf, capacity) != 0) {
            CPE_ERROR(
                agent->m_em, "%s: create pkg buf for data size %d fail",
                center_agent_name(agent), (int)capacity);
            return NULL;
        }
    }

    return mem_buffer_make_continuous(&agent->m_incoming_pkg_buf, 0);
}

static void center_agent_center_on_read(center_agent_t agent, net_ep_t ep) {
    size_t curent_pkg_size = 1024;
    uint32_t i;

    for(i = 0; i < agent->m_process_count_per_tick; ++i) {
        char * buf;
        size_t buf_size;
        size_t input_size;
        dr_cvt_result_t cvt_result;
        void * req_buf;
        size_t req_size;
        SVR_CENTER_PKG * pkg;

        buf_size = net_ep_size(ep);
        if (buf_size <= 0) break;

        buf = net_ep_peek(ep, NULL, buf_size);
        if (buf == NULL) {
            CPE_ERROR(
                agent->m_em, "%s: center ep %d: peek data fail, size=%d!",
                center_agent_name(agent), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }

        input_size = buf_size;

    RESIZE_AND_TRY_AGAIN:
        req_buf = center_agent_get_incoming_pkg_buff(agent, curent_pkg_size);
        if (req_buf == NULL) {
            CPE_ERROR(agent->m_em, "%s: center ep %d: get pkg buf fail!", center_agent_name(agent), (int)net_ep_id(ep))
            net_ep_close(ep);
            break;
        }
        bzero(req_buf, curent_pkg_size);

        req_size = curent_pkg_size;
        cvt_result =
            dr_cvt_decode(
                agent->m_cvt, agent->m_pkg_meta,
                req_buf, &req_size,
                buf, &input_size,
                agent->m_em, agent->m_debug >= 2 ? 1 : 0);
        if (cvt_result == dr_cvt_result_not_enough_input) {
            if (curent_pkg_size >= agent->m_max_pkg_size) {
                CPE_ERROR(
                    agent->m_em, "%s: center ep %d: not enough data, input size is %d, buf size is %d!",
                    center_agent_name(agent), (int)net_ep_id(ep), (int)buf_size, (int)curent_pkg_size);
                net_ep_close(ep);
                break;
            }
            else {
                curent_pkg_size *= 2;
                if (curent_pkg_size > agent->m_max_pkg_size) curent_pkg_size = agent->m_max_pkg_size;
                goto RESIZE_AND_TRY_AGAIN;
            }
        }
        else if (cvt_result != dr_cvt_result_success) {
            CPE_ERROR(
                agent->m_em, "%s: center ep %d: decode package fail, input size is %d!",
                center_agent_name(agent), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }
        net_ep_erase(ep, input_size);

        if(agent->m_debug >= 2) {
            CPE_INFO(
                agent->m_em, "%s: center ep %d: decode one package, buf-origin-size=%d left-size=%d!",
                center_agent_name(agent), (int)net_ep_id(ep), (int)buf_size, (int)net_ep_size(ep));
        }

        if (agent->m_debug) {
            struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&agent->m_dump_buffer);
            mem_buffer_clear_data(&agent->m_dump_buffer);

            dr_json_print((write_stream_t)&stream, req_buf, req_size, agent->m_pkg_meta, 0, NULL);
            stream_putc((write_stream_t)&stream, 0);

            CPE_ERROR(
                agent->m_em,
                "%s: center ep %d: d: <== recv on request\n%s",
                center_agent_name(agent), (int)net_ep_id(ep),
                (const char *)mem_buffer_make_continuous(&agent->m_dump_buffer, 0));
        }

        pkg = req_buf;
        if (pkg->cmd >= sizeof(g_center_agent_center_ops) / sizeof(g_center_agent_center_ops[0])
            || g_center_agent_center_ops[pkg->cmd] == NULL)
        {
            CPE_ERROR(
                agent->m_em, "%s: center ep %d: cmd %d: no processor of cmd!",
                center_agent_name(agent), net_ep_id(ep), pkg->cmd);
            return;
        }

        (*g_center_agent_center_ops[pkg->cmd])(agent, pkg, req_size);
    }

    if(agent->m_debug >= 2) {
        CPE_INFO(
            agent->m_em, "%s: center ep %d: on read, process %d requiest(s)",
            center_agent_name(agent), (int)net_ep_id(ep), (int)i);
    }
}

static void center_agent_center_on_open(center_agent_t agent, net_ep_t ep) {
    SVR_CENTER_PKG pkg;

    printf("open 1\n");
    if(agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: center ep %d: on open",
            center_agent_name(agent), (int)net_ep_id(ep));
    }
    printf("open 2\n");

    pkg.cmd = SVR_CENTER_CMD_REQ_JOIN;
    pkg.data.svr_center_req_join.id.svr_type = 0;
    pkg.data.svr_center_req_join.id.svr_id = 0;

    if (center_agent_center_send(agent, &pkg, sizeof(pkg)) != 0) {
        CPE_ERROR(
            agent->m_em, "%s: center ep %d: send login req fail!",
            center_agent_name(agent), (int)net_ep_id(ep));
        net_ep_close(ep);
    }
    else {
        if (agent->m_debug) {
            CPE_INFO(
                agent->m_em, "%s: center ep %d: send join!",
                center_agent_name(agent), (int)net_ep_id(ep));
        }
        agent->m_center_state = center_agent_center_state_registing;
    }
}

static void center_agent_center_on_close(center_agent_t agent, net_ep_t ep, net_ep_event_t event) {
    if(agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: center ep %d: on close, event=%d",
            center_agent_name(agent), (int)net_ep_id(ep), event);
    }

    agent->m_center_pkg_send_time = 0;
    agent->m_center_state = center_agent_center_state_init;
}

static void center_agent_center_process(net_ep_t ep, void * ctx, net_ep_event_t event) {
    center_agent_t agent = (center_agent_t)ctx;

    assert(agent);

    switch(event) {
    case net_ep_event_read:
        center_agent_center_on_read(agent, ep);
        break;
    case net_ep_event_open:
        center_agent_center_on_open(agent, ep);
        break;
    default:
        center_agent_center_on_close(agent, ep, event);
        break;
    }
}

static void center_agent_free_chanel_buf(net_chanel_t chanel, void * ctx) {
    center_agent_t agent = (center_agent_t)ctx;

    assert(agent);

    mem_free(agent->m_alloc, net_chanel_queue_buf(chanel));
}

int center_agent_center_ep_init(center_agent_t agent, net_ep_t ep) {
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

    net_ep_set_processor(ep, center_agent_center_process, agent);

    return 0;
EP_INIT_ERROR:
    if (buf_r) mem_free(agent->m_alloc, buf_r);
    if (buf_w) mem_free(agent->m_alloc, buf_w);
    if (chanel_r) net_chanel_free(chanel_r);
    if (chanel_w) net_chanel_free(chanel_w);
    net_ep_close(ep);

    CPE_ERROR(
        agent->m_em, "%s: center ep %d: init fail!",
        center_agent_name(agent), (int)net_ep_id(ep));

    return -1;
}
