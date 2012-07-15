#include "cpe/utils/error.h"
#include "cpe/dp/dp_manage.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_bind/bpg_bind_manage.h"
#include "bpg_bind_internal_types.h"

int bpg_bind_manage_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    bpg_bind_manage_t mgr = (bpg_bind_manage_t)ctx;
    bpg_pkg_t pkg;

    /* if (req->type == "control") { */
    /*     if (connect) {// */
    /*     } */
    /*     else if (disconnect) { */
    /*         /\*close queue*\/ */
    /*     } */
    /* } */
    /* else { */
        pkg = bpg_pkg_from_dp_req(req);
        if (pkg == NULL) {
            /* CPE_ERROR( */
            /*     mgr->m_em, "%s: ep %d: dispatch cmd %d error!", */
            /*     bpg_net_mgr_name(mgr), (int)net_ep_id(ep), bpg_pkg_cmd(req_buf)); */
            return -1;
        }

        /* if (new_connection) { */
        /*     find_old; */
        /*     build control pkg */
        /*     if (dp_dispatch_by_string(mgr->m_reply_to, control_pkg, agent->m_em) != 0) { */
        /*     } */

        /* } */

        if (dp_dispatch_by_numeric(bpg_pkg_cmd(pkg), bpg_pkg_to_dp_req(pkg), mgr->m_em) != 0) {
            CPE_ERROR(
                mgr->m_em, "%s: dispatch cmd %d error!",
                bpg_bind_manage_name(mgr), bpg_pkg_cmd(pkg));
            return -1;
        }
    /* } */

    return 0;
}
