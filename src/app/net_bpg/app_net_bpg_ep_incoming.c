#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "app/net_pkg/app_net_pkg.h"
#include "app/net_bpg/app_net_bpg_ep.h"
#include "app_net_bpg_internal_ops.h"

int app_net_bpg_ep_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    app_net_bpg_ep_t agent = ctx;
    bpg_pkg_t req_buf;
    app_net_pkg_t app_net_pkg;
    size_t buf_size;
    size_t input_size;
    dr_cvt_result_t cvt_result;

    app_net_pkg = app_net_pkg_from_dp_req(req);
    if (app_net_pkg == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: recv req type error!",
            app_net_bpg_ep_name(agent));
        return -1;
    }

    if(agent->m_debug >= 2) {
        CPE_INFO(
            agent->m_em, "%s: app-type %d: recv",
            app_net_bpg_ep_name(agent), (int)app_net_pkg_app_type(app_net_pkg));
    }

    req_buf = app_net_bpg_ep_req_buf(agent);
    if (req_buf == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: app-type %d: get req buf fail!",
            app_net_bpg_ep_name(agent), (int)app_net_pkg_app_type(app_net_pkg));
        return -1;
    }

    buf_size = app_net_pkg_data_size(app_net_pkg);

    input_size = buf_size;

    cvt_result =
        bpg_pkg_decode(
            req_buf,
            app_net_pkg_data(app_net_pkg), &input_size, agent->m_em, agent->m_debug >= 2 ? 1 : 0);
    if (cvt_result != dr_cvt_result_success) {
        CPE_ERROR(
            agent->m_em, "%s: app-type %d: decode package fail, input size is %d!",
            app_net_bpg_ep_name(agent), (int)app_net_pkg_app_type(app_net_pkg), (int)buf_size);
        return -1;
    }

    if(agent->m_debug >= 2) {
        CPE_INFO(
            agent->m_em, "%s: app-type %d: decode one package, buf-origin-size=%d!",
            app_net_bpg_ep_name(agent), (int)app_net_pkg_app_type(app_net_pkg), (int)buf_size);
    }

    if (agent->m_debug) {
        CPE_ERROR(
            agent->m_em,
            "%s: <== app_net_bpg_ep_recv: recv one pkg!\n"
            "%s",
            app_net_bpg_ep_name(agent),
            bpg_pkg_dump(req_buf, &agent->m_dump_buffer));
    }

    if (dp_dispatch_by_string(agent->m_dispatch_to, bpg_pkg_to_dp_req(req_buf), agent->m_em) != 0) {
        CPE_ERROR(
            agent->m_em, "%s: app-type %d: dispatch cmd %d error!",
            app_net_bpg_ep_name(agent), (int)app_net_pkg_app_type(app_net_pkg), bpg_pkg_cmd(req_buf));
    }

    return 0;
}
