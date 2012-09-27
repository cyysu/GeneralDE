#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/buffer.h"
#include "cpe/dp/dp_manage.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "app/net_pkg/app_net_pkg.h"
#include "app/net_proxy/app_net_ep.h"
#include "app/net_bpg/app_net_bpg_ep.h"
#include "app_net_bpg_internal_ops.h"

int app_net_bpg_ep_send(dp_req_t req, void * ctx, error_monitor_t em) {
    app_net_bpg_ep_t ep;
    bpg_pkg_t pkg;
    size_t write_size;
    dr_cvt_result_t cvt_result;
    app_net_pkg_t outgoing_buf;

    ep = (app_net_bpg_ep_t)ctx;
    pkg = bpg_pkg_from_dp_req(req);

    if (pkg == NULL) {
        CPE_ERROR(
            em, "%s: app_net_bpg_ep_reply: input req is not bpg_pkg!",
            app_net_bpg_ep_name(ep));
        return 0;
    }

    if (ep->m_debug) {
        CPE_ERROR(
            ep->m_em,
            "%s: app_net_bpg_ep_send: send one request!\n"
            "%s",
            app_net_bpg_ep_name(ep),
            bpg_pkg_dump(pkg, &ep->m_dump_buffer));
    }

    outgoing_buf = app_net_bpg_ep_outgoing_buf(ep);
    if (outgoing_buf == NULL) {
        CPE_ERROR(
            ep->m_em,
            "%s: app_net_bpg_ep_send: get outgoing buf fail!!",
            app_net_bpg_ep_name(ep));
        return 0;
    }

    write_size = app_net_pkg_data_capacity(outgoing_buf);
    cvt_result =
        bpg_pkg_encode(
            pkg,
            app_net_pkg_data(outgoing_buf), &write_size,
            ep->m_em, ep->m_debug >= 2 ? 1 : 0);
    if (cvt_result != dr_cvt_result_success) {
        CPE_ERROR(
            ep->m_em, "%s: app_net_bpg_ep_send: encode package for send fail!",
            app_net_bpg_ep_name(ep));
        return 0;
    }

    if (app_net_ep_send(ep->m_ep, outgoing_buf) != 0) {
        CPE_ERROR(
            ep->m_em, "%s: app_net_bpg_ep_send: send data fail, write_size=" FMT_SIZE_T "!",
            app_net_bpg_ep_name(ep), write_size);
        return 0;
    }

    if (bpg_pkg_sn(pkg) != INVALID_LOGIC_REQUIRE_ID) {
        if (logic_require_queue_add(ep->m_require_queue, bpg_pkg_sn(pkg)) != 0) {
            CPE_INFO(
                ep->m_em, "%s: app_net_bpg_ep_send: save require id fail!",
                app_net_bpg_ep_name(ep));
        }
    }

    if (ep->m_debug) {
        CPE_ERROR(
            ep->m_em,
            "%s: app_net_bpg_ep_send: send one request, write-size=" FMT_SIZE_T "!",
            app_net_bpg_ep_name(ep), write_size);
    }

    return 0;
}

