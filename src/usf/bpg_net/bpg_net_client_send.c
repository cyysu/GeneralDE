#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_cvt.h"
#include "cpe/utils/buffer.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/net/net_connector.h"
#include "usf/logic/logic_require.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_net/bpg_net_client.h"
#include "bpg_net_internal_ops.h"

int bpg_net_client_send(dp_req_t req, void * ctx, error_monitor_t em) {
    bpg_net_client_t client;
    bpg_pkg_t pkg;
    size_t pkg_size;
    size_t write_size;
    net_ep_t ep;
    dr_cvt_result_t cvt_result;

    client = (bpg_net_client_t)ctx;
    pkg = bpg_pkg_from_dp_req(req);

    if (pkg == NULL) {
        CPE_ERROR(
            em, "%s: bpg_net_client_reply: input req is not bpg_pkg!",
            bpg_net_client_name(client));
        return 0;
    }

    if (client->m_debug) {
        struct mem_buffer buffer;
        mem_buffer_init(&buffer, NULL);

        CPE_ERROR(
            client->m_em,
            "%s: bpg_net_client_send: send one request!\n"
            "%s",
            bpg_net_client_name(client),
            bpg_pkg_dump(pkg, &buffer));

        mem_buffer_clear(&buffer);
    }

    assert(client->m_logic_mgr);

    if (net_connector_state(client->m_connector) != net_connector_state_connected) {
        CPE_ERROR(
            client->m_em,
            "%s: bpg_net_client_send: network not connected, state=%d!\n",
            bpg_net_client_name(client),
            net_connector_state(client->m_connector));
        if (bpg_pkg_sn(pkg) != INVALID_LOGIC_REQUIRE_ID) {
            logic_require_t require = logic_require_find(client->m_logic_mgr, bpg_pkg_sn(pkg));
            if (require == NULL) {
                if (client->m_debug) {
                    CPE_ERROR(
                        client->m_em,
                        "%s: bpg_net_client_send: network not connected, notify require %u fail, require not exist!\n",
                        bpg_net_client_name(client), bpg_pkg_sn(pkg));
                }
            }
            else {
                logic_require_set_error(require);
                if (client->m_debug) {
                    CPE_INFO(
                        client->m_em,
                        "%s: bpg_net_client_send: network not connected, notify require %u error complete!\n",
                        bpg_net_client_name(client), bpg_pkg_sn(pkg));
                }
            }
        }

        return 0;
    }

    ep = net_connector_ep(client->m_connector);

    mem_buffer_set_size(&client->m_send_encode_buf, client->m_req_max_size);
    pkg_size = bpg_pkg_pkg_data_size(pkg);
    write_size = mem_buffer_size(&client->m_send_encode_buf);
    cvt_result =
        dr_cvt_encode(
            bpg_pkg_base_cvt(pkg),
            bpg_pkg_base_meta(pkg),
            mem_buffer_make_continuous(&client->m_send_encode_buf, 0),
            &write_size,
            bpg_pkg_pkg_data(pkg),
            &pkg_size,
            client->m_em, client->m_debug >= 2 ? 1 : 0);
    if (cvt_result != dr_cvt_result_success) {
        CPE_ERROR(
            client->m_em, "%s: bpg_net_client_send: encode package for send fail!",
            bpg_net_client_name(client));
        return 0;
    }

    if (net_ep_send(ep, mem_buffer_make_continuous(&client->m_send_encode_buf, 0), write_size) != 0) {
        CPE_ERROR(
            client->m_em, "%s: bpg_net_client_send: send data fail, write_size=" FMT_SIZE_T "!",
            bpg_net_client_name(client), write_size);
        net_ep_close(ep);
        return 0;
    }

    if (bpg_pkg_sn(pkg) != INVALID_LOGIC_REQUIRE_ID) {
        if (bpg_net_client_save_require_id(client, bpg_pkg_sn(pkg)) != 0) {
            CPE_INFO(
                client->m_em, "%s: bpg_net_client_send: save require id fail!",
                bpg_net_client_name(client));
        }
    }

    if (client->m_debug) {
        CPE_ERROR(
            client->m_em,
            "%s: bpg_net_client_send: send one request, write-size=" FMT_SIZE_T "!",
            bpg_net_client_name(client), write_size);
    }

    return 0;
}

