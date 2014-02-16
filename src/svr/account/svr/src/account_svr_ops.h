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
    mongo_id_generator_t id_generator,
    mem_allocrator_t alloc,
    error_monitor_t em);

void account_svr_free(account_svr_t svr);

account_svr_t account_svr_find(gd_app_context_t app, cpe_hash_string_t name);
account_svr_t account_svr_find_nc(gd_app_context_t app, const char * name);
const char * account_svr_name(account_svr_t svr);

/*util pos */
int account_svr_validate_passwd(
    account_svr_t svr, SVR_ACCOUNT_BASIC const * account,
    SVR_ACCOUNT_LOGIC_ID const * logic_id, const char * passwd);

/*conn svr ops*/
int account_svr_is_conn_svr(account_svr_t svr, uint16_t svr_type_id);
int account_svr_conn_bind_account(account_svr_t svr, logic_context_t ctx, uint16_t conn_svr_id, uint16_t conn_svr_type, uint64_t account_id);
int account_svr_conn_get_conn_info(account_svr_t svr, logic_context_t ctx, uint32_t * conn_id, uint64_t * account_id);

/*db ops*/
int account_svr_db_send_query_by_logic_id(
    account_svr_t svr, logic_require_t require,
    SVR_ACCOUNT_LOGIC_ID const * logic_id, LPDRMETA result_meta);
int account_svr_db_send_insert(
    account_svr_t svr, logic_require_t require, 
    uint64_t account_id, const char * passwd, SVR_ACCOUNT_LOGIC_ID const * logic_id);
int account_svr_db_send_bind(
    account_svr_t svr, logic_require_t require, 
    uint64_t account_id, SVR_ACCOUNT_LOGIC_ID const * logic_id);
int account_svr_db_send_unbind(
    account_svr_t svr, logic_require_t require, 
    uint64_t account_id, uint16_t account_type);

/*service ops*/
logic_op_exec_result_t
account_svr_op_create_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_create_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
account_svr_op_login_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_login_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

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

logic_op_exec_result_t
account_svr_op_query_by_account_id_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_query_by_account_id_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);


#endif
