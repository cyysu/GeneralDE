#include <assert.h>
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/net/net_connector.h"
#include "cpe/net/net_endpoint.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

int center_agent_center_send(center_agent_t agent, SVR_CENTER_PKG * pkg, size_t pkg_size) {
    net_ep_t ep = net_connector_ep(agent->m_connector);
    size_t curent_pkg_size = mem_buffer_size(&agent->m_outgoing_encode_buf);
    void * encode_buf;
    size_t buf_size;
    size_t input_size;
    dr_cvt_result_t cvt_result;

    if (curent_pkg_size < 1024) curent_pkg_size = 1024;

    do {
        if (mem_buffer_size(&agent->m_outgoing_encode_buf) < curent_pkg_size) {
            if (mem_buffer_set_size(&agent->m_outgoing_encode_buf, curent_pkg_size) != 0) {
                CPE_ERROR(
                    agent->m_em, "%s: center ep %d: send: set encode buf to size %d fail",
                    center_agent_name(agent), net_ep_id(ep),
                    (int)curent_pkg_size);
                return -1;
            }
        }

        encode_buf = mem_buffer_make_continuous(&agent->m_outgoing_encode_buf, 0);
        assert(encode_buf);
        buf_size = mem_buffer_size(&agent->m_outgoing_encode_buf);
        input_size = pkg_size;

        cvt_result =
            dr_cvt_encode(
                agent->m_cvt, agent->m_pkg_meta,
                encode_buf, &buf_size,
                pkg, &input_size,
                agent->m_em, agent->m_debug >= 2 ? 1 : 0);
        if (cvt_result == dr_cvt_result_not_enough_input) {
            if (curent_pkg_size >= agent->m_max_pkg_size) {
                CPE_ERROR(
                    agent->m_em, "%s: center ep %d: send: not enough encode buf, buf size is %d!",
                    center_agent_name(agent), net_ep_id(ep),
                    (int)curent_pkg_size);
                return -1;
            }
            else {
                curent_pkg_size *= 2;
                if (curent_pkg_size > agent->m_max_pkg_size) curent_pkg_size = agent->m_max_pkg_size;
                continue;
            }
        }
        else if (cvt_result != dr_cvt_result_success) {
            CPE_ERROR(
                agent->m_em, "%s: center ep %d: send: encode fail, buf size is %d!",
                center_agent_name(agent), net_ep_id(ep),
                (int)curent_pkg_size);
            return -1;
        }

        if (net_ep_send(ep, encode_buf, buf_size) != 0) {
            CPE_ERROR(
                agent->m_em, "%s: center ep %d: send: send to net fail, send-size=%d!",
                center_agent_name(agent), net_ep_id(ep),
                (int)buf_size);
            return -1;
        }

        if (agent->m_debug) {
            struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&agent->m_dump_buffer);
            mem_buffer_clear_data(&agent->m_dump_buffer);

            dr_json_print((write_stream_t)&stream, pkg, pkg_size, agent->m_pkg_meta, 0, NULL);
            stream_putc((write_stream_t)&stream, 0);

            CPE_ERROR(
                agent->m_em, "%s: center ep %d: ==> send on request, send-size=%d\n%s",
                center_agent_name(agent), (int)net_ep_id(ep),
                (int)buf_size,
                (const char *)mem_buffer_make_continuous(&agent->m_dump_buffer, 0));
        }

        break;
    } while(0);

    return 0;
}
