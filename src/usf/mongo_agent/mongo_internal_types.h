#ifndef TSF4WG_TCAPLUS_INTERNAL_TYPES_H
#define TSF4WG_TCAPLUS_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/dp/dp_types.h"
#include "usf/mongo_agent/mongo_types.h"

struct mongo_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    logic_manage_t m_logic_mgr;

    char m_password[64];

    uint32_t m_runing_require_capacity;
    uint32_t m_runing_require_count;
    uint32_t m_runing_require_op_count;
    uint32_t m_runing_require_check_span;
    logic_require_id_t * m_runing_requires;

    struct mem_buffer m_dump_buffer;
    uint32_t m_dump_buffer_capacity;

    int m_debug;
};

#endif
