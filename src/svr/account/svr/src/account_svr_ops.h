#ifndef SVR_ACCOUNT_SVR_OPS_H
#define SVR_ACCOUNT_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "account_svr_types.h"
#include "svr/center/agent/center_agent_types.h" 
#include "protocol/svr/account/svr_account_pro.h"

/*operations of account_svr */
account_svr_t
account_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_sp_t set_sp,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mem_allocrator_t alloc,
    error_monitor_t em);

void account_svr_free(account_svr_t svr);

account_svr_t account_svr_find(gd_app_context_t app, cpe_hash_string_t name);
account_svr_t account_svr_find_nc(gd_app_context_t app, const char * name);
const char * account_svr_name(account_svr_t svr);

/*ops*/
logic_op_exec_result_t
account_svr_op_bind_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_bind_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
account_svr_op_unbind_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_unbind_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
account_svr_op_query_by_logic_id_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_query_by_logic_id_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

#endif
