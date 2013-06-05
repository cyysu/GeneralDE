#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/hash_string.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/buffer.h"
#include "cpe/net/net_endpoint.h"
#include "gd/vnet/vnet_control_pkg.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_net/bpg_net_agent.h"
#include "bpg_net_internal_ops.h"

static int bpg_net_agent_send_to_client(bpg_net_agent_t agent, bpg_pkg_t pkg, error_monitor_t em) {
    size_t write_size;
    net_ep_t ep;
    dr_cvt_result_t cvt_result;

    ep = net_ep_find(gd_app_net_mgr(agent->m_app), bpg_pkg_connection_id(pkg));
    if (ep == NULL) {
        CPE_ERROR(
            em, "%s: bpg_net_agent_reply: no connection associate with %d!",
            bpg_net_agent_name(agent), bpg_pkg_connection_id(pkg));
        return 0;
    }

    if (agent->m_debug) {
        CPE_ERROR(
            agent->m_em,
            "%s: ep %d: ==> send on reply\n%s",
            bpg_net_agent_name(agent), (int)net_ep_id(ep), bpg_pkg_dump(pkg, &agent->m_dump_buffer));
    }
    
    mem_buffer_set_size(&agent->m_rsp_buf, agent->m_req_max_size);
    write_size = mem_buffer_size(&agent->m_rsp_buf);
    cvt_result =
        bpg_pkg_encode(
            pkg,
            mem_buffer_make_continuous(&agent->m_rsp_buf, 0),
            &write_size,
            agent->m_em, agent->m_debug >= 2 ? 1 : 0);
    if (cvt_result != dr_cvt_result_success) {
        CPE_ERROR(
            agent->m_em, "%s: bpg_net_agent_reply: encode package for send fail!",
            bpg_net_agent_name(agent));

        if (agent->m_debug) {
            CPE_INFO(agent->m_em, "\n\n");
        }

        return 0;
    }

    if (net_ep_send(ep, mem_buffer_make_continuous(&agent->m_rsp_buf, 0), write_size) != 0) {
        CPE_ERROR(
            agent->m_em, "%s: bpg_net_agent_reply: send data fail, write_size=%d!",
            bpg_net_agent_name(agent), (int)write_size);
        net_ep_close(ep);

        if (agent->m_debug) {
            CPE_INFO(agent->m_em, "\n\n");
        }

        return 0;
    }
    
    if (agent->m_debug >= 2) {
        CPE_ERROR(
            agent->m_em,
            "%s: bpg_net_agent_reply: send one response, write-size=" FMT_SIZE_T " !\n\n",
            bpg_net_agent_name(agent), write_size);
    }

    return 0;
}

static int bpg_net_agent_process_control_pkg(bpg_net_agent_t agent, vnet_control_pkg_t pkg, error_monitor_t em) {
    net_ep_t ep;

    ep = net_ep_find(gd_app_net_mgr(agent->m_app), vnet_control_pkg_connection_id(pkg));
    if (ep == NULL) {
        CPE_ERROR(
            em, "%s: bpg_net_agent_process_control_pkg: no connection associate with %d!",
            bpg_net_agent_name(agent), vnet_control_pkg_connection_id(pkg));
        return 0;
    }

    switch(vnet_control_pkg_cmd(pkg)) {
    case vnet_control_op_disconnect:
        net_ep_set_status(ep, NET_REMOVE_AFTER_SEND);
        break;
    case vnet_control_op_ignore_input:
        break;
    }

    return 0;
}

int bpg_net_agent_reply(dp_req_t req, void * ctx, error_monitor_t em) {
    bpg_net_agent_t agent;
    bpg_pkg_t pkg;

    agent = (bpg_net_agent_t)ctx;

    if (cpe_hs_cmp(dp_req_type_hs(req), vnet_control_pkg_type_name) == 0) {
        return bpg_net_agent_process_control_pkg(
            agent, vnet_control_pkg_from_dp_req(req), em);
    }
    else {
        pkg = bpg_pkg_from_dp_req(req);

        if (pkg == NULL) {
            CPE_ERROR(
                em, "%s: bpg_net_agent_reply: input req is not bpg_pkg!",
                bpg_net_agent_name(agent));

            if (agent->m_debug) {
                CPE_INFO(agent->m_em, "\n\n");
            }

            return 0;
        }

        return bpg_net_agent_send_to_client(agent, pkg, em);
    }
}
