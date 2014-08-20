#ifndef SVR_VERSION_SVR_TYPES_H
#define SVR_VERSION_SVR_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/version/svr_version_pro.h"

typedef struct version_svr * version_svr_t;

struct version_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    cpe_hash_string_t m_send_to;
    dp_rsp_t m_recv_at;

    LPDRMETA m_meta_res_query_update;
    LPDRMETA m_meta_res_error;
};

#endif
