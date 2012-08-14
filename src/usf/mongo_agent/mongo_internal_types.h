#ifndef TSF4WG_TCAPLUS_INTERNAL_TYPES_H
#define TSF4WG_TCAPLUS_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/dp/dp_types.h"
#include "usf/mongo_agent/mongo_types.h"

TAILQ_HEAD(mongo_table_list, mongo_table);

struct mongo_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    logic_manage_t m_logic_mgr;

    char m_password[64];

    struct mongo_table_list m_tables;

    struct cpe_hash_table m_requests;
    struct cpe_hash_table m_results;

    uint32_t m_runing_require_capacity;
    uint32_t m_runing_require_count;
    uint32_t m_runing_require_op_count;
    uint32_t m_runing_require_check_span;
    logic_require_id_t * m_runing_requires;

    struct mem_buffer m_dump_buffer;
    uint32_t m_dump_buffer_capacity;

    LPDRMETALIB m_result_metalib;
    struct mem_buffer m_result_metalib_buffer;
    
    int m_debug;
};

struct mongo_table {
    mongo_agent_t m_agent;
    const char * m_name;
    LPDRMETA m_meta;
    LPDRMETA m_result_meta;
    LPDRMETALIB m_metalib;
    size_t m_data_capacity;

    TAILQ_ENTRY(mongo_table) m_next;
};

struct mongo_request {
    mongo_agent_t m_agent;
    const char * m_name;
    mongo_table_t m_table;

    struct cpe_hash_entry m_hh;
};

struct mongo_result {
    mongo_agent_t m_agent;
    const char * m_name;
    mongo_table_t m_table;
    logic_data_t m_data;

    struct cpe_hash_entry m_hh;
};

#endif
