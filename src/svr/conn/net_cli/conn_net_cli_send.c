#include <assert.h>
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_request.h"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "svr/conn/net_cli/conn_net_cli_pkg.h"
#include "svr/conn/net_cli/conn_net_cli_svr_stub.h"
#include "protocol/svr/conn/svr_conn_net.h"
#include "conn_net_cli_internal_ops.h"

static int conn_net_cli_send_i(conn_net_cli_t cli, conn_net_cli_svr_stub_t svr_stub, uint32_t sn, LPDRMETA meta, void const * data, uint16_t data_len);

int conn_net_cli_send(conn_net_cli_t cli, uint16_t to_svr, uint32_t sn, LPDRMETA meta, void const * data, uint16_t data_len) {
    conn_net_cli_svr_stub_t svr_stub;

    svr_stub = conn_net_cli_svr_stub_find_by_id(cli, to_svr);
    if (svr_stub == NULL) {
        CPE_ERROR(cli->m_em, "%s: send: svr_stub %d is unknown!", conn_net_cli_name(cli), to_svr);
        return -1;
    }

    return conn_net_cli_send_i(cli, svr_stub, sn, meta, data, data_len);
}

static int conn_net_cli_send_i(conn_net_cli_t cli, conn_net_cli_svr_stub_t svr_stub, uint32_t sn, LPDRMETA meta, void const * data, uint16_t data_len) {
    size_t curent_pkg_size = 2048;
    ringbuffer_block_t blk;
    SVR_CONN_NET_REQ_HEAD * head;
    void * buf;

    if (meta && svr_stub->m_pkg_meta && meta != svr_stub->m_pkg_meta) {
        CPE_ERROR(
            cli->m_em, "%s: send: svr %s(%d) pkg meta is %s, but input is %s",
            conn_net_cli_name(cli), svr_stub->m_svr_type_name, svr_stub->m_svr_type_id,
            dr_meta_name(svr_stub->m_pkg_meta), dr_meta_name(meta));
        return -1;
    }

    if (meta == NULL) meta = svr_stub->m_pkg_meta;
    if (meta == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: send: svr %s(%d) no meta configured",
            conn_net_cli_name(cli), svr_stub->m_svr_type_name, svr_stub->m_svr_type_id);
        return -1;
    }

    while(curent_pkg_size < data_len) { 
        curent_pkg_size *= 2;
    }

RESIZE_AND_TRY_AGAIN:
    blk = ringbuffer_alloc(cli->m_ringbuf , curent_pkg_size);
    if (blk == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: send: not enouth ringbuf, curent_pkg_size=%d, data-len=%d!",
            conn_net_cli_name(cli), (int)curent_pkg_size, (int)data_len);
    }

    buf = NULL;
    ringbuffer_data(cli->m_ringbuf, blk, curent_pkg_size, 0, &buf);
    assert(buf);

    head = buf;
    head->to_svr = svr_stub->m_svr_type_id;
    head->sn = sn;

    if (data) {
        int encode_size;

        encode_size =
            dr_pbuf_write(
                head + 1,
                curent_pkg_size - sizeof(SVR_CONN_NET_REQ_HEAD),
                data, data_len, meta, cli->m_em);
        if (encode_size < 0) {
            if (encode_size == dr_code_error_not_enough_output) {
                if (curent_pkg_size < cli->m_max_pkg_size) {
                    curent_pkg_size *= 2;

                    if (cli->m_debug) {
                        CPE_INFO(
                            cli->m_em, "%s: send: encode require buf not enouth, resize to %d, input_data_len=%d",
                            conn_net_cli_name(cli), (int)curent_pkg_size, (int)data_len);
                    }

                    ringbuffer_free(cli->m_ringbuf, blk);
                    goto RESIZE_AND_TRY_AGAIN;
                }
                else {
                    CPE_ERROR(
                        cli->m_em, "%s: send: encode require buf too big!, curent_pkg_size=%d, input_data_len=%d",
                        conn_net_cli_name(cli), (int)curent_pkg_size, (int)data_len);
                    ringbuffer_free(cli->m_ringbuf, blk);
                    return -1;
                }
            }
            else {
                CPE_ERROR(
                    cli->m_em, "%s: send: encode fail!, curent_pkg_size=%d, input_data_len=%d",
                    conn_net_cli_name(cli), (int)curent_pkg_size, (int)data_len);
                ringbuffer_free(cli->m_ringbuf, blk);
                return -1;
            }
        }
        else {
            head->pkg_len = sizeof(SVR_CONN_NET_REQ_HEAD) + encode_size;
        }
    }
    else {
        head->pkg_len = sizeof(SVR_CONN_NET_REQ_HEAD);
    }
    

    ringbuffer_shrink(cli->m_ringbuf, blk, head->pkg_len);
    conn_net_cli_link_node_w(cli, blk);

    if (fsm_machine_curent_state(&cli->m_fsm) == conn_net_cli_state_established) {
        assert(cli->m_fd != -1);
        ev_io_stop(cli->m_ev_loop, &cli->m_watcher);
        conn_net_cli_start_watch(cli);
    }

    return 0;
}

int conn_net_cli_svr_stub_outgoing_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    conn_net_cli_svr_stub_t svr_stub = ctx;
    conn_net_cli_t cli = svr_stub->m_cli;
    conn_net_cli_pkg_t pkg;

    pkg = conn_net_cli_pkg_find(req);
    if (pkg == NULL) {
        CPE_ERROR(cli->m_em, "%s: send: cast req to pkg fail!", conn_net_cli_name(cli));
        return -1;
    }

    return conn_net_cli_send_i(svr_stub->m_cli, svr_stub, pkg->m_sn, dp_req_meta(req), dp_req_data(req), dp_req_size(req));
}
