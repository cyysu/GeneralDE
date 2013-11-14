#ifndef SVR_DBLOG_SVR_INTERNAL_OPS_H
#define SVR_DBLOG_SVR_INTERNAL_OPS_H
#include "cpe/utils/hash_string.h"
#include "svr/center/agent/center_agent_types.h" 
#include "dblog_agent_internal_types.h"

dblog_agent_t
dblog_agent_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    uint16_t dblog_svr_type_id,
    mem_allocrator_t alloc,
    error_monitor_t em);

void dblog_agent_free(dblog_agent_t svr);


#endif
