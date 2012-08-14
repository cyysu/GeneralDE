#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "usf/mongo_agent/mongo_table.h"
#include "mongo_internal_ops.h"

int mongo_agent_send_request(mongo_agent_t agent, mongo_request_t request, logic_require_t require) {
    if (require) {
        if (mongo_agent_save_require_id(agent, logic_require_id(require)) != 0) {
            CPE_INFO(
                agent->m_em, "%s: send_request: save require id fail!",
                mongo_agent_name(agent));
        }
    }

    return 0;
}

ptr_int_t mongo_agent_tick(void * ctx, ptr_int_t max_process_count) {
    mongo_agent_t agent;
    uint32_t i;

    agent = (mongo_agent_t)ctx;
    return 0;

    if (agent->m_debug >= 4) {
        CPE_INFO(
            gd_app_em(agent->m_app), "%s: tick",
            mongo_agent_name(agent));
    }

    for(i = 0; i < max_process_count; ++i) {
    }

    return i;
}

