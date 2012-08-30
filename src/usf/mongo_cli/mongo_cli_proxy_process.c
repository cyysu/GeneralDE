#include <assert.h>
#include "cpe/dp/dp_manage.h"
#include "usf/logic/logic_require.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "mongo_cli_internal_ops.h"

int mongo_cli_proxy_send(mongo_cli_proxy_t agent, mongo_pkg_t pkg, logic_require_t require) {
    if (agent->m_outgoing_send_to == NULL) {
        CPE_INFO(
            agent->m_em, "%s: send: no outgoing_send_to configured",
            mongo_cli_proxy_name(agent));
        goto ERROR;
    }

    if (require) mongo_pkg_set_id(pkg, logic_require_id(require));

    if (dp_dispatch_by_string(agent->m_outgoing_send_to, mongo_pkg_to_dp_req(pkg), agent->m_em) != 0) {
        CPE_INFO(agent->m_em, "%s: send_request: dispatch return fail!", mongo_cli_proxy_name(agent));
        goto ERROR;
    }

    if (logic_require_queue_add(agent->m_require_queue, logic_require_id(require)) != 0) {
        CPE_INFO(agent->m_em, "%s: send_request: save require id fail!", mongo_cli_proxy_name(agent));
        goto ERROR;
    }

    return 0;

ERROR:
    if (require) logic_require_error(require);
    return -1;
}

int mongo_cli_proxy_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    struct mongo_cli_proxy * proxy;
    mongo_pkg_t pkg;
    uint32_t sn;
    logic_require_t require;
 
    proxy = (struct mongo_cli_proxy *)ctx;

    pkg = mongo_pkg_from_dp_req(req);
    if (pkg == NULL) {
        CPE_ERROR(em, "mongo_cli_proxy_rsp: cast to pkg fail!");
        return -1;
    }

    sn = mongo_pkg_id(pkg);
    require = logic_require_queue_remove_get(proxy->m_require_queue, sn);
    if (require == NULL) {
        CPE_ERROR(em, "mongo_cli_proxy_rsp: require %d not exist in queue!", sn);
        return -1;
    }

    /* if (bpg_cli_proxy_save_pkg_info(require, pkg, em) != 0 */
    /*     || bpg_cli_proxy_save_main_body(require, pkg, em) != 0 */
    /*     || bpg_cli_proxy_save_append_infos(require, pkg, em) != 0 */
    /*     ) */
    /* { */
    /*     logic_require_set_error(require); */
    /*     return -1; */
    /* } */

    logic_require_set_done(require);
    return 0;
}
