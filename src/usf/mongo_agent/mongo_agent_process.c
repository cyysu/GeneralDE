#include <assert.h>
#include "cpe/dp/dp_manage.h"
#include "usf/logic/logic_require.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "mongo_agent_internal_ops.h"

int mongo_agent_send(mongo_agent_t agent, mongo_pkg_t pkg, logic_require_t require) {
    if (agent->m_outgoing_send_to == NULL) {
        CPE_INFO(
            agent->m_em, "%s: send: no outgoing_send_to configured",
            mongo_agent_name(agent));
        goto ERROR;
    }

    if (require) mongo_pkg_set_id(pkg, logic_require_id(require));

    if (dp_dispatch_by_string(agent->m_outgoing_send_to, mongo_pkg_to_dp_req(pkg), agent->m_em) != 0) {
        CPE_INFO(agent->m_em, "%s: send_request: dispatch return fail!", mongo_agent_name(agent));
        goto ERROR;
    }

    if (logic_require_queue_add_require_id(agent->m_require_queue, logic_require_id(require)) != 0) {
        CPE_INFO(agent->m_em, "%s: send_request: save require id fail!", mongo_agent_name(agent));
        goto ERROR;
    }

    return 0;

ERROR:
    if (require) logic_require_error(require);
    return -1;
}

int mongo_agent_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    return 0;
}
