#ifndef SVR_DBLOG_AGENT_TYPES_H
#define SVR_DBLOG_AGENT_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/pal/pal_socket.h"
#include "gd/timer/timer_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "svr/dblog/agent/dblog_agent_types.h"

struct dblog_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;
};

#endif
