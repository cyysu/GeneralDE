#ifndef USF_MONGO_AGENT_INTERNAL_TYPES_H
#define USF_MONGO_AGENT_INTERNAL_TYPES_H
#include "usf/logic/logic_types.h"
#include "usf/mongo_agent/mongo_agent_types.h"

struct mongo_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    logic_manage_t m_logic_mgr;
    mongo_driver_t m_driver;

    uint32_t m_runing_require_capacity;
    uint32_t m_runing_require_count;
    uint32_t m_runing_require_op_count;
    uint32_t m_runing_require_check_span;
    logic_require_id_t * m_runing_requires;

    int m_debug;
};

#endif
