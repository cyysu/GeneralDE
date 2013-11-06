#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/dblog/agent/dblog_agent.h"
#include "dblog_agent_internal_ops.h"

EXPORT_DIRECTIVE
int dblog_agent_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    dblog_agent_t dblog_agent;

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    dblog_agent = dblog_agent_create(app, gd_app_module_name(module), stub, gd_app_alloc(app), gd_app_em(app));
    if (dblog_agent == NULL) return -1;

    dblog_agent->m_debug = cfg_get_int8(cfg, "debug", dblog_agent->m_debug);

    if (dblog_agent->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void dblog_agent_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dblog_agent_t dblog_agent;

    dblog_agent = dblog_agent_find_nc(app, gd_app_module_name(module));
    if (dblog_agent) {
        dblog_agent_free(dblog_agent);
    }
}
