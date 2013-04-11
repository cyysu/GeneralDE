#include <assert.h> 
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/net/net_endpoint.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "center_svr_ops.h"

center_cli_conn_t
center_cli_conn_create(center_svr_t svr, net_ep_t ep) {
    center_cli_conn_t conn;

    assert(ep);

    conn = mem_alloc(svr->m_alloc, sizeof(struct center_cli_conn));
    if (conn == NULL) {
        CPE_ERROR(svr->m_em, "%s: create conn: malloc fail!", center_svr_name(svr));
        return NULL;
    }

    conn->m_svr = svr;
    conn->m_ep = ep;
    conn->m_data = NULL;

    TAILQ_INSERT_TAIL(&svr->m_conns, conn, m_next);

    if(svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: conn %d: create", center_svr_name(svr), (int)net_ep_id(conn->m_ep));
    }

    return conn;
}

void center_cli_conn_free(center_cli_conn_t conn) {
    center_svr_t svr = conn->m_svr;
    assert(svr);
    assert(conn->m_ep);

    if(svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: conn %d: free", center_svr_name(svr), (int)net_ep_id(conn->m_ep));
    }

    TAILQ_REMOVE(&svr->m_conns, conn, m_next);

    if (conn->m_data) {
        conn->m_data->m_conn = NULL;
        conn->m_data = NULL;
    }

    net_ep_free(conn->m_ep);
    conn->m_ep = NULL;

    mem_free(svr->m_alloc, conn);
}

void center_cli_conn_free_all(center_svr_t svr) {
    while(!TAILQ_EMPTY(&svr->m_conns)) {
        center_cli_conn_free(TAILQ_FIRST(&svr->m_conns));
    }
}

int center_cli_conn_id(center_cli_conn_t conn) {
    return net_ep_id(conn->m_ep);
}

int center_cli_conn_send(center_cli_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size) {
    center_svr_t svr = conn->m_svr;
    size_t curent_pkg_size = mem_buffer_size(&svr->m_outgoing_encode_buf);
    void * encode_buf;
    size_t buf_size;
    size_t input_size;
    dr_cvt_result_t cvt_result;

    if (curent_pkg_size < 1024) curent_pkg_size = 1024;

    do {
        if (mem_buffer_size(&svr->m_outgoing_encode_buf) < curent_pkg_size) {
            if (mem_buffer_set_size(&svr->m_outgoing_encode_buf, curent_pkg_size) != 0) {
                CPE_ERROR(
                    svr->m_em, "%s: ep %d: svr %d.%d: send: set encode buf to size %d fail",
                    center_svr_name(svr), net_ep_id(conn->m_ep),
                    conn->m_data ? conn->m_data->m_data->svr_type : 0,
                    conn->m_data ? conn->m_data->m_data->svr_id : 0,
                    (int)curent_pkg_size);
                return -1;
            }
        }

        encode_buf = mem_buffer_make_continuous(&svr->m_outgoing_encode_buf, 0);
        assert(encode_buf);
        buf_size = mem_buffer_size(&svr->m_outgoing_encode_buf);
        input_size = pkg_size;

        cvt_result =
            dr_cvt_encode(
                svr->m_cvt, svr->m_pkg_meta,
                encode_buf, &buf_size,
                pkg, &input_size,
                svr->m_em, svr->m_debug >= 2 ? 1 : 0);
        if (cvt_result == dr_cvt_result_not_enough_input) {
            if (curent_pkg_size >= svr->m_max_pkg_size) {
                CPE_ERROR(
                    svr->m_em, "%s: ep %d: svr %d.%d: send: not enough encode buf, buf size is %d!",
                    center_svr_name(svr), net_ep_id(conn->m_ep),
                    conn->m_data ? conn->m_data->m_data->svr_type : 0,
                    conn->m_data ? conn->m_data->m_data->svr_id : 0,
                    (int)curent_pkg_size);
                return -1;
            }
            else {
                curent_pkg_size *= 2;
                if (curent_pkg_size > svr->m_max_pkg_size) curent_pkg_size = svr->m_max_pkg_size;
                continue;
            }
        }
        else if (cvt_result != dr_cvt_result_success) {
            CPE_ERROR(
                svr->m_em, "%s: ep %d: svr %d.%d: send: encode fail, buf size is %d!",
                center_svr_name(svr), net_ep_id(conn->m_ep),
                conn->m_data ? conn->m_data->m_data->svr_type : 0,
                conn->m_data ? conn->m_data->m_data->svr_id : 0,
                (int)curent_pkg_size);
            return -1;
        }

        if (net_ep_send(conn->m_ep, encode_buf, buf_size) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: ep %d: svr %d.%d: send: send to net fail, send-size=%d!",
                center_svr_name(svr), net_ep_id(conn->m_ep),
                conn->m_data ? conn->m_data->m_data->svr_type : 0,
                conn->m_data ? conn->m_data->m_data->svr_id : 0,
                (int)buf_size);
            return -1;
        }

        if (svr->m_debug) {
            struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&svr->m_dump_buffer);
            mem_buffer_clear_data(&svr->m_dump_buffer);

            dr_json_print((write_stream_t)&stream, pkg, pkg_size, svr->m_pkg_meta, 0, NULL);
            stream_putc((write_stream_t)&stream, 0);

            CPE_ERROR(
                svr->m_em, "%s: ep %d: svr %d.%d: ==> send on request, send-size=%d\n%s",
                center_svr_name(svr), (int)net_ep_id(conn->m_ep),
                conn->m_data ? conn->m_data->m_data->svr_type : 0,
                conn->m_data ? conn->m_data->m_data->svr_id : 0,
                (int)buf_size,
                (const char *)mem_buffer_make_continuous(&svr->m_dump_buffer, 0));
        }

        break;
    } while(0);

    return 0;
}
