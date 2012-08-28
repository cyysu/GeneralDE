#ifndef USF_MONGO_AGENT_INTERNAL_TYPES_H
#define USF_MONGO_AGENT_INTERNAL_TYPES_H
#include "cpe/dp/dp_types.h"
#include "usf/logic/logic_types.h"
#include "usf/logic_use/logic_use_types.h"
#include "usf/mongo_agent/mongo_agent_types.h"

struct mongo_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    logic_require_queue_t m_require_queue;

    cpe_hash_string_t m_outgoing_send_to;
    dp_rsp_t m_incoming_recv_at;
    
    int m_debug;
};

#endif
