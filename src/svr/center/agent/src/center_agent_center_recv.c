#include <assert.h>
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/net/net_connector.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

void * center_agent_get_incoming_pkg_buff(struct center_agent_center * center, size_t capacity) {
    if (mem_buffer_size(&center->m_incoming_pkg_buf) < capacity) {
        if (mem_buffer_set_size(&center->m_incoming_pkg_buf, capacity) != 0) {
            CPE_ERROR(
                center->m_agent->m_em, "%s: create pkg buf for data size %d fail",
                center_agent_name(center->m_agent), (int)capacity);
            return NULL;
        }
    }

    return mem_buffer_make_continuous(&center->m_incoming_pkg_buf, 0);
}

static void center_agent_center_on_read(struct center_agent_center * center, net_ep_t ep) {
    size_t curent_pkg_size = 1024;
    uint32_t i;

    for(i = 0; i < center->m_process_count_per_tick; ++i) {
        char * buf;
        size_t buf_size;
        size_t input_size;
        dr_cvt_result_t cvt_result;
        void * req_buf;
        size_t req_size;

        buf_size = net_ep_size(ep);
        if (buf_size <= 0) break;

        buf = net_ep_peek(ep, NULL, buf_size);
        if (buf == NULL) {
            CPE_ERROR(
                center->m_agent->m_em, "%s: center ep %d: peek data fail, size=%d!",
                center_agent_name(center->m_agent), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }

        input_size = buf_size;

    RESIZE_AND_TRY_AGAIN:
        req_buf = center_agent_get_incoming_pkg_buff(center, curent_pkg_size);
        if (req_buf == NULL) {
            CPE_ERROR(
                center->m_agent->m_em, "%s: center ep %d: get pkg buf fail!",
                center_agent_name(center->m_agent), (int)net_ep_id(ep))
            net_ep_close(ep);
            break;
        }
        bzero(req_buf, curent_pkg_size);

        req_size = curent_pkg_size;
        cvt_result =
            dr_cvt_decode(
                center->m_cvt, center->m_pkg_meta,
                req_buf, &req_size,
                buf, &input_size,
                center->m_agent->m_em, center->m_agent->m_debug >= 2 ? 1 : 0);
        if (cvt_result == dr_cvt_result_not_enough_input) {
            if (curent_pkg_size >= center->m_max_pkg_size) {
                CPE_ERROR(
                    center->m_agent->m_em, "%s: center ep %d: not enough data, input size is %d, buf size is %d!",
                    center_agent_name(center->m_agent), (int)net_ep_id(ep), (int)buf_size, (int)curent_pkg_size);
                net_ep_close(ep);
                break;
            }
            else {
                curent_pkg_size *= 2;
                if (curent_pkg_size > center->m_max_pkg_size) curent_pkg_size = center->m_max_pkg_size;
                goto RESIZE_AND_TRY_AGAIN;
            }
        }
        else if (cvt_result != dr_cvt_result_success) {
            CPE_ERROR(
                center->m_agent->m_em, "%s: center ep %d: decode package fail, input size is %d!",
                center_agent_name(center->m_agent), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }
        net_ep_erase(ep, input_size);

        if(center->m_agent->m_debug >= 2) {
            CPE_INFO(
                center->m_agent->m_em, "%s: center ep %d: decode one package, buf-origin-size=%d left-size=%d!",
                center_agent_name(center->m_agent), (int)net_ep_id(ep), (int)buf_size, (int)net_ep_size(ep));
        }

        if (center->m_agent->m_debug) {
            struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&center->m_agent->m_dump_buffer);
            mem_buffer_clear_data(&center->m_agent->m_dump_buffer);

            dr_json_print((write_stream_t)&stream, req_buf, req_size, center->m_pkg_meta, 0, NULL);
            stream_putc((write_stream_t)&stream, 0);

            CPE_INFO(
                center->m_agent->m_em,
                "%s: center ep %d: d: <== recv one response\n%s",
                center_agent_name(center->m_agent), (int)net_ep_id(ep),
                (const char *)mem_buffer_make_continuous(&center->m_agent->m_dump_buffer, 0));
        }

        center_agent_center_apply_pkg(center, req_buf);
    }

    if(center->m_agent->m_debug >= 2) {
        CPE_INFO(
            center->m_agent->m_em, "%s: center ep %d: on read, process %d requiest(s)",
            center_agent_name(center->m_agent), (int)net_ep_id(ep), (int)i);
    }
}

static void center_agent_center_on_open(struct center_agent_center * center, net_ep_t ep) {
    if(center->m_agent->m_debug) {
        CPE_INFO(
            center->m_agent->m_em, "%s: center ep %d: on open",
            center_agent_name(center->m_agent), (int)net_ep_id(ep));
    }
}

static void center_agent_center_on_close(struct center_agent_center * center, net_ep_t ep, net_ep_event_t event) {
    if(center->m_agent->m_debug) {
        CPE_INFO(
            center->m_agent->m_em, "%s: center ep %d: on close, event=%d",
            center_agent_name(center->m_agent), (int)net_ep_id(ep), event);
    }
}

static void center_agent_center_process(net_ep_t ep, void * ctx, net_ep_event_t event) {
    struct center_agent_center * center = ctx;

    assert(center);

    switch(event) {
    case net_ep_event_read:
        center_agent_center_on_read(center, ep);
        break;
    case net_ep_event_open:
        center_agent_center_on_open(center, ep);
        break;
    default:
        center_agent_center_on_close(center, ep, event);
        break;
    }
}

static void center_agent_center_free_chanel_buf(net_chanel_t chanel, void * ctx) {
    struct center_agent_center * center = ctx;

    assert(center);

    mem_free(center->m_agent->m_alloc, net_chanel_queue_buf(chanel));
}

void center_agent_center_connector_state_monitor(net_connector_t connector, void * ctx) {
    struct center_agent_center * center = ctx;

    switch(net_connector_state(connector)) {
    case net_connector_state_connected: {
        center_agent_center_apply_evt(center, center_agent_fsm_evt_connected);
        break;
    }
    case net_connector_state_error: {
        center_agent_center_apply_evt(center, center_agent_fsm_evt_disconnected);
        break;
    }
    case net_connector_state_disable:
    case net_connector_state_idle:
    case net_connector_state_connecting:
    default:
        if(center->m_agent->m_debug) {
            CPE_INFO(
                center->m_agent->m_em, "%s: connector monitor: state=%s", 
                center_agent_name(center->m_agent),
                net_connector_state_str(net_connector_state(connector)));
        }
        break;
    }
}

int center_agent_center_ep_init(struct center_agent_center * center, net_ep_t ep) {
    void * buf_r = NULL;
    void * buf_w = NULL;
    net_chanel_t chanel_r = NULL;
    net_chanel_t chanel_w = NULL;

    assert(center);

    buf_r = mem_alloc(center->m_agent->m_alloc, center->m_read_chanel_size);
    buf_w = mem_alloc(center->m_agent->m_alloc, center->m_write_chanel_size);
    if (buf_r == NULL || buf_w == NULL) goto EP_INIT_ERROR;

    chanel_r = net_chanel_queue_create(net_ep_mgr(ep), buf_r, center->m_read_chanel_size);
    if (chanel_r == NULL) goto EP_INIT_ERROR;
    net_chanel_queue_set_close(chanel_r, center_agent_center_free_chanel_buf, center);
    buf_r = NULL;

    chanel_w = net_chanel_queue_create(net_ep_mgr(ep), buf_w, center->m_write_chanel_size);
    if (chanel_w == NULL) goto EP_INIT_ERROR;
    net_chanel_queue_set_close(chanel_w, center_agent_center_free_chanel_buf, center);
    buf_w = NULL;

    net_ep_set_chanel_r(ep, chanel_r);
    chanel_r = NULL;

    net_ep_set_chanel_w(ep, chanel_w);
    chanel_w = NULL;

    net_ep_set_processor(ep, center_agent_center_process, center);

    return 0;
EP_INIT_ERROR:
    if (buf_r) mem_free(center->m_agent->m_alloc, buf_r);
    if (buf_w) mem_free(center->m_agent->m_alloc, buf_w);
    if (chanel_r) net_chanel_free(chanel_r);
    if (chanel_w) net_chanel_free(chanel_w);
    net_ep_close(ep);

    CPE_ERROR(
        center->m_agent->m_em, "%s: center ep %d: init fail!",
        center_agent_name(center->m_agent), (int)net_ep_id(ep));

    return -1;
}
