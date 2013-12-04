#ifndef USF_MONGO_CLI_INTERNAL_TYPES_H
#define USF_MONGO_CLI_INTERNAL_TYPES_H
#include "cpe/dp/dp_types.h"
#include "usf/logic/logic_types.h"
#include "usf/logic_use/logic_use_types.h"
#include "usf/mongo_cli/mongo_cli_types.h"

struct mongo_cli_proxy {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    mongo_driver_t m_driver;
    logic_require_queue_t m_require_queue;

    cpe_hash_string_t m_outgoing_send_to;
    dp_rsp_t m_incoming_recv_at;
    
    size_t m_pkg_buf_max_size;
    mongo_pkg_t m_pkg_buf;
    mongo_pkg_t m_cmd_buf;
    char m_dft_db[64];

    LPDRMETA m_meta_lasterror;
    LPDRMETA m_meta_cmd_info;
    LPDRMETA m_meta_result_build_info;

    int m_debug;
};

#endif
