#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "center_svr_ops.h"
#include "protocol/svr/center/svr_center_pro.h"

static center_cli_conn_op_t g_center_cli_ops[] = {
    NULL
    , center_cli_conn_op_join, NULL
    , center_cli_conn_op_query_by_type, NULL
};

static void center_cli_conn_on_read(center_cli_conn_t conn, net_ep_t ep) {
    center_svr_t svr = conn->m_svr;
    size_t curent_pkg_size = 1024;
    uint32_t i;

    for(i = 0; i < svr->m_process_count_per_tick; ++i) {
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
                svr->m_em, "%s: ep %d: peek data fail, size=%d!",
                center_svr_name(svr), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }

        input_size = buf_size;

    RESIZE_AND_TRY_AGAIN:
        req_buf = center_svr_get_incoming_pkg_buff(svr, curent_pkg_size);
        if (req_buf == NULL) {
            CPE_ERROR(svr->m_em, "%s: ep %d: get pkg buf fail!", center_svr_name(svr), (int)net_ep_id(ep))
            net_ep_close(ep);
            break;
        }
        bzero(req_buf, curent_pkg_size);

        req_size = curent_pkg_size;
        cvt_result =
            dr_cvt_decode(
                svr->m_cvt, svr->m_pkg_meta,
                req_buf, &req_size,
                buf, &input_size,
                svr->m_em, svr->m_debug >= 2 ? 1 : 0);
        if (cvt_result == dr_cvt_result_not_enough_input) {
            if (curent_pkg_size >= svr->m_max_pkg_size) {
                CPE_ERROR(
                    svr->m_em, "%s: ep %d: not enough data, input size is %d, buf size is %d!",
                    center_svr_name(svr), (int)net_ep_id(ep), (int)buf_size, (int)curent_pkg_size);
                net_ep_close(ep);
                break;
            }
            else {
                curent_pkg_size *= 2;
                if (curent_pkg_size > svr->m_max_pkg_size) curent_pkg_size = svr->m_max_pkg_size;
                goto RESIZE_AND_TRY_AGAIN;
            }
        }
        else if (cvt_result != dr_cvt_result_success) {
            CPE_ERROR(
                svr->m_em, "%s: ep %d: decode package fail, input size is %d!",
                center_svr_name(svr), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }
        net_ep_erase(ep, input_size);

        if(svr->m_debug >= 2) {
            CPE_INFO(
                svr->m_em, "%s: ep %d: decode one package, buf-origin-size=%d left-size=%d!",
                center_svr_name(svr), (int)net_ep_id(ep), (int)buf_size, (int)net_ep_size(ep));
        }

        if (svr->m_debug) {
            struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&svr->m_dump_buffer);
            mem_buffer_clear_data(&svr->m_dump_buffer);

            dr_json_print((write_stream_t)&stream, req_buf, req_size, svr->m_pkg_meta, 0, NULL);
            stream_putc((write_stream_t)&stream, 0);

            CPE_INFO(
                svr->m_em,
                "%s: ep %d: d: svr %d.%d: <== recv one request\n%s",
                center_svr_name(svr), (int)net_ep_id(ep),
                conn->m_data ? conn->m_data->m_data->svr_type : 0,
                conn->m_data ? conn->m_data->m_data->svr_id : 0,
                (const char *)mem_buffer_make_continuous(&svr->m_dump_buffer, 0));
        }

        pkg = req_buf;
        if (pkg->cmd >= sizeof(g_center_cli_ops) / sizeof(g_center_cli_ops[0])
            || g_center_cli_ops[pkg->cmd] == NULL)
        {
            CPE_ERROR(
                svr->m_em, "%s: ep %d: svr %d.%d: cmd %d: no processor of cmd!",
                center_svr_name(svr), net_ep_id(conn->m_ep),
                conn->m_data ? conn->m_data->m_data->svr_type : 0,
                conn->m_data ? conn->m_data->m_data->svr_id : 0,
                pkg->cmd);
            return;
        }

        (*g_center_cli_ops[pkg->cmd])(conn, pkg, req_size);
    }

    if(svr->m_debug >= 2) {
        CPE_INFO(
            svr->m_em, "%s: ep %d: on read, process %d requiest(s)",
            center_svr_name(svr), (int)net_ep_id(ep), (int)i);
    }
}

static void center_cli_conn_on_open(center_cli_conn_t conn, net_ep_t ep) {
    center_svr_t svr = conn->m_svr;

    if(svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: ep %d: on open",
            center_svr_name(svr), (int)net_ep_id(ep));
    }
}

static void center_cli_conn_on_close(center_cli_conn_t conn, net_ep_t ep, net_ep_event_t event) {
    center_svr_t svr = conn->m_svr;

    if(svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: ep %d: on close, event=%d",
            center_svr_name(svr), (int)net_ep_id(ep), event);
    }

    center_cli_conn_free(conn);
}

static void center_cli_conn_process(net_ep_t ep, void * ctx, net_ep_event_t event) {
    center_cli_conn_t conn = (center_cli_conn_t)ctx;

    assert(conn);

    switch(event) {
    case net_ep_event_read:
        center_cli_conn_on_read(conn, ep);
        break;
    case net_ep_event_open:
        center_cli_conn_on_open(conn, ep);
        break;
    default:
        center_cli_conn_on_close(conn, ep, event);
        break;
    }
}

static void center_svr_free_chanel_buf(net_chanel_t chanel, void * ctx) {
    center_svr_t svr = (center_svr_t)ctx;

    assert(svr);

    mem_free(svr->m_alloc, net_chanel_queue_buf(chanel));
}


void center_svr_accept(net_listener_t listener, net_ep_t ep, void * ctx) {
    center_svr_t svr = (center_svr_t)ctx;
    center_cli_conn_t conn = NULL;
    void * buf_r = NULL;
    void * buf_w = NULL;
    net_chanel_t chanel_r = NULL;
    net_chanel_t chanel_w = NULL;

    assert(svr);

    buf_r = mem_alloc(svr->m_alloc, svr->m_read_chanel_size);
    buf_w = mem_alloc(svr->m_alloc, svr->m_write_chanel_size);
    if (buf_r == NULL || buf_w == NULL) goto ACCEPT_ERROR;

    chanel_r = net_chanel_queue_create(net_ep_mgr(ep), buf_r, svr->m_read_chanel_size);
    if (chanel_r == NULL) goto ACCEPT_ERROR;
    net_chanel_queue_set_close(chanel_r, center_svr_free_chanel_buf, svr);
    buf_r = NULL;

    chanel_w = net_chanel_queue_create(net_ep_mgr(ep), buf_w, svr->m_write_chanel_size);
    if (chanel_w == NULL) goto ACCEPT_ERROR;
    net_chanel_queue_set_close(chanel_w, center_svr_free_chanel_buf, svr);
    buf_w = NULL;

    net_ep_set_chanel_r(ep, chanel_r);
    chanel_r = NULL;

    net_ep_set_chanel_w(ep, chanel_w);
    chanel_w = NULL;

    conn = center_cli_conn_create(svr, ep);
    if (conn == NULL) {
        CPE_ERROR(svr->m_em, "%s: ep %d: create center_cli_conn fail!", center_svr_name(svr), (int)net_ep_id(ep));
        goto ACCEPT_ERROR;
    }

    net_ep_set_processor(ep, center_cli_conn_process, conn);
    net_ep_set_timeout(ep, svr->m_conn_timeout_ms);

    if(svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: ep %d: accept success!",
            center_svr_name(svr), (int)net_ep_id(ep));
    }

    return;
ACCEPT_ERROR:
    if (buf_r) mem_free(svr->m_alloc, buf_r);
    if (buf_w) mem_free(svr->m_alloc, buf_w);
    if (chanel_r) net_chanel_free(chanel_r);
    if (chanel_w) net_chanel_free(chanel_w);
    net_ep_close(ep);

    CPE_ERROR(
        svr->m_em, "%s: ep %d: accept fail!",
        center_svr_name(svr), (int)net_ep_id(ep));
}

