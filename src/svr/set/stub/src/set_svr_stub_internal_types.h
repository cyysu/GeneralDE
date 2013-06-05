#ifndef SVR_SET_STUB_INTERNAL_TYPES_H
#define SVR_SET_STUB_INTERNAL_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/net/net_types.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/timer/timer_manage.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "protocol/svr/set/svr_set_pro.h"

struct set_svr_stub_dispach_info {
    cpe_hash_string_t m_notify_dispatch_to;
    cpe_hash_string_t m_response_dispatch_to;
};

struct set_svr_stub {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
    center_agent_t m_agent;

    center_agent_svr_type_t m_svr_type;
    uint16_t m_svr_id;
    uint32_t m_process_count_per_tick;

    cpe_hash_string_t m_request_dispatch_to;
    cpe_hash_string_t m_response_dispatch_to;
    size_t m_dispatch_info_count;
    struct set_svr_stub_dispach_info * m_dispatch_infos;

    dp_rsp_t m_outgoing_recv_at;

    set_chanel_t m_chanel;

    dp_req_t m_incoming_buf;

    struct mem_buffer m_dump_buffer_head;
    struct mem_buffer m_dump_buffer_carry;
    struct mem_buffer m_dump_buffer_body;
};

#endif
