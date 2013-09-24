#ifndef SVR_PAYMENT_SVR_OPS_H
#define SVR_PAYMENT_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "payment_svr_types.h"
#include "svr/center/agent/center_agent_types.h" 
#include "protocol/svr/payment/svr_payment_pro.h"

/*operations of payment_svr */
payment_svr_t
payment_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_sp_t set_sp,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mem_allocrator_t alloc,
    error_monitor_t em);

void payment_svr_free(payment_svr_t svr);

payment_svr_t payment_svr_find(gd_app_context_t app, cpe_hash_string_t name);
payment_svr_t payment_svr_find_nc(gd_app_context_t app, const char * name);
const char * payment_svr_name(payment_svr_t svr);

/*mongo_pkg utils function*/
int payment_svr_mongo_pkg_append_id(mongo_pkg_t db_pkg, uint64_t uid, uint16_t bag_id);
int payment_svr_mongo_pkg_append_required_moneies(mongo_pkg_t db_pkg, uint8_t mongo_type_count);

/*bag_info*/
BAG_INFO * payment_svr_meta_bag_info_find(payment_svr_t svr, uint16_t bag_id);
int payment_svr_meta_bag_info_load(payment_svr_t svr, cfg_t cfg);

/*product_info*/
PRODUCT_INFO * payment_svr_meta_product_info_find(payment_svr_t svr, const char * product_id);
int payment_svr_meta_product_info_load(payment_svr_t svr, cfg_t cfg);

/*ops*/
logic_op_exec_result_t
payment_svr_op_get_balance_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
payment_svr_op_get_balance_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
payment_svr_op_pay_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
payment_svr_op_pay_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
payment_svr_op_recharge_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
payment_svr_op_recharge_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

/*util functions*/
int payment_svr_op_validate_money_types(payment_svr_t svr, BAG_INFO * bag_info, SVR_PAYMENT_MONEY_GROUP const * moneies);

/*recharge*/
logic_op_exec_result_t
payment_svr_op_recharge_db_update(
    logic_context_t ctx, logic_stack_node_t stack,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, BAG_INFO * bag_info);

/*db ops*/
int payment_svr_db_send_query_money(
    payment_svr_t svr, BAG_INFO * bag_info, logic_require_t require,
    uint64_t user_id);

int payment_svr_db_send_add_money(
    payment_svr_t svr, BAG_INFO * bag_info, logic_require_t require,
    uint64_t user_id, SVR_PAYMENT_MONEY_GROUP const * diff);

int payment_svr_db_send_remove_money(
    payment_svr_t svr, BAG_INFO * bag_info, logic_require_t require,
    uint64_t user_id, SVR_PAYMENT_MONEY_GROUP const * diff);

int payment_svr_db_send_init_money(
    payment_svr_t svr, BAG_INFO * bag_info, logic_require_t require,
    uint64_t user_id, SVR_PAYMENT_MONEY_GROUP const * diff);

int payment_svr_db_build_balance(
    payment_svr_t svr, BAG_INFO * bag_info, logic_require_t require,
    SVR_PAYMENT_MONEY_GROUP * balance);

int payment_svr_db_validate_result(payment_svr_t svr, logic_require_t require);

void payment_svr_db_add_bill(
    payment_svr_t svr, BAG_INFO * bag_info, uint64_t user_id, 
    PAYMENT_BILL_DATA const * bill_data, SVR_PAYMENT_MONEY_GROUP const * balance);

int payment_svr_find_count_by_type(uint64_t * result, SVR_PAYMENT_MONEY_GROUP const * monies, uint8_t money_type);
uint64_t payment_svr_get_count_by_type(SVR_PAYMENT_MONEY_GROUP const * monies, uint8_t money_type, uint64_t dft);

/*gift recharge operations*/
logic_op_exec_result_t
payment_svr_op_recharge_gift_send(
    logic_context_t ctx, logic_stack_node_t stack,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info);

/*iap recharge operations*/
logic_op_exec_result_t
payment_svr_op_recharge_iap_send(
    logic_context_t ctx, logic_stack_node_t stack,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info);

logic_op_exec_result_t
payment_svr_op_recharge_iap_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info);

#endif
