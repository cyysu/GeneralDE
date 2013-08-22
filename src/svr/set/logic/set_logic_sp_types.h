#ifndef SVR_SET_LOGIC_INTERNAL_TYPES_H
#define SVR_SET_LOGIC_INTERNAL_TYPES_H
#include "usf/logic_use/logic_use_types.h"
#include "svr/set/logic/set_logic_types.h"

struct set_logic_sp {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    LPDRMETA m_pkg_info_meta;

    logic_require_queue_t m_require_queue;
    dp_req_t m_outgoing_pkg;
    dp_req_t m_outgoing_body;

    cpe_hash_string_t m_outgoing_dispatch_to;
    dp_rsp_t m_incoming_recv_at;

};

#endif
