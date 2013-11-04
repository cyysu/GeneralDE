#ifndef SVR_DBLOG_SVR_TYPES_H
#define SVR_DBLOG_SVR_TYPES_H
#include "ev.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/pal/pal_socket.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"

typedef struct dblog_svr * dblog_svr_t;

struct dblog_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;
    struct ev_loop * m_ev_loop;

    int m_fd;
    ev_io m_watcher;
};

#endif
