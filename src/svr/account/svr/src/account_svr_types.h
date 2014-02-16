#ifndef SVR_ACCOUNT_SVR_TYPES_H
#define SVR_ACCOUNT_SVR_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "gd/timer/timer_types.h"
#include "usf/logic_use/logic_use_types.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "usf/mongo_use/mongo_use_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "svr/set/logic/set_logic_types.h"
#include "protocol/svr/account/svr_account_internal.h"

typedef struct account_svr * account_svr_t;

struct account_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    set_logic_sp_t m_set_sp;
    int m_debug;

    logic_op_register_t m_op_register;
    set_logic_rsp_manage_t m_rsp_manage;
    mongo_cli_proxy_t m_db;
    mongo_id_generator_t m_id_generator;

    LPDRMETA m_meta_res_login;
    LPDRMETA m_meta_res_create;
    LPDRMETA m_meta_logic_id;
    LPDRMETA m_meta_record_full;
    LPDRMETA m_meta_record_full_list;
    LPDRMETA m_meta_record_basic;
    LPDRMETA m_meta_record_basic_list;

    struct mem_buffer m_dump_buffer;
};

#endif
