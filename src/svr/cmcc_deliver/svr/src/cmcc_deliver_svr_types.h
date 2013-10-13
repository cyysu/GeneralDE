#ifndef SVR_CMCC_DELIVER_SVR_TYPES_H
#define SVR_CMCC_DELIVER_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/cmcc_deliver/svr_cmcc_deliver_internal.h"

typedef struct cmcc_deliver_svr * cmcc_deliver_svr_t;

struct cmcc_deliver_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    LPDRMETA m_pkg_meta_res_deliver;

    cpe_hash_string_t m_send_to;
    dp_rsp_t m_recv_at;
};

typedef void (*cmcc_deliver_svr_op_t)(cmcc_deliver_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

#endif
