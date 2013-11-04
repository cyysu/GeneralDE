#ifndef SVR_DBLOG_SVR_OPS_H
#define SVR_DBLOG_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "dblog_svr_types.h"
#include "svr/center/agent/center_agent_types.h" 

/*operations of dblog_svr */
dblog_svr_t
dblog_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em);

void dblog_svr_free(dblog_svr_t svr);

dblog_svr_t dblog_svr_find(gd_app_context_t app, cpe_hash_string_t name);
dblog_svr_t dblog_svr_find_nc(gd_app_context_t app, const char * name);
const char * dblog_svr_name(dblog_svr_t svr);

void dblog_svr_net_cb(EV_P_ ev_io *w, int revents);

#endif
