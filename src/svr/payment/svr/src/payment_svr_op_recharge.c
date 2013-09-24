#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "usf/mongo_cli/mongo_cli_result.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "payment_svr_ops.h"
#include "protocol/svr/payment/svr_payment_pro.h"
#include "protocol/svr/payment/svr_payment_internal.h"

/*支持的充值操作列表 */
static struct {
    logic_op_exec_result_t (*send)(
        logic_context_t ctx, logic_stack_node_t stack,
        payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info);
    logic_op_exec_result_t (*recv)(
        logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
        payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info);
} g_way_processors[] =
{
    { NULL, NULL }
    , { payment_svr_op_recharge_gift_send, NULL }
    , { payment_svr_op_recharge_iap_send, payment_svr_op_recharge_iap_recv }
};

static logic_op_exec_result_t
payment_svr_op_recharge_on_db_update(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info);

static logic_op_exec_result_t
payment_svr_op_recharge_on_db_insert(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info);

logic_op_exec_result_t
payment_svr_op_recharge_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_PAYMENT_REQ_RECHARGE const * req;
    BAG_INFO * bag_info;
    logic_data_t res_data;
    SVR_PAYMENT_RES_RECHARGE * res;

    /*获取请求 */
    req_data = logic_context_data_find(ctx, "svr_payment_req_recharge");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: get request fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    /*检查bag_id*/
    bag_info = payment_svr_meta_bag_info_find(svr, req->bag_id);
    if (bag_info == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: bag info of %d not exist!", payment_svr_name(svr), req->bag_id);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_BAG_ID_ERROR);
        return logic_op_exec_result_false;
    }
 
    /*初始化一个空的返回结果 */
    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_recharge, sizeof(SVR_PAYMENT_RES_RECHARGE));
    if (res_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: create response buf fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);
    res->result = 0;
    res->way_result = 0;
    res->balance.count = 0;

    /*根据充值类型分别处理 */
    if (req->way <= 0 || req->way >= (sizeof(g_way_processors) / sizeof(g_way_processors[0]))) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: way %d is unknown!", payment_svr_name(svr), req->way);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_RECHARGE_WAY_ERROR);
        return logic_op_exec_result_false;
    }

    assert(g_way_processors[req->way].send);
    return g_way_processors[req->way].send(ctx, stack, svr, req, res, bag_info);
}

logic_op_exec_result_t
payment_svr_op_recharge_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    SVR_PAYMENT_REQ_RECHARGE const * req;
    SVR_PAYMENT_RES_RECHARGE * res;
    BAG_INFO * bag_info;

    req = logic_data_data(logic_context_data_find(ctx, "svr_payment_req_recharge"));
    assert(req);

    bag_info = payment_svr_meta_bag_info_find(svr, req->bag_id);
    assert(bag_info);
 
    res = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_res_recharge)));
    assert(res);

    if (strcmp(logic_require_name(require), "recharge_update") == 0) {
        return payment_svr_op_recharge_on_db_update(ctx, stack, require, svr, req, res, bag_info);
    }
    else if (strcmp(logic_require_name(require), "recharge_insert") == 0) {
        return payment_svr_op_recharge_on_db_insert(ctx, stack, require, svr, req, res, bag_info);
    }
    else {
        assert(req->way > 0 && req->way < (sizeof(g_way_processors) / sizeof(g_way_processors[0])));

        if (g_way_processors[req->way].recv) {
            return g_way_processors[req->way].recv(ctx, stack, require, svr, req, res, bag_info);
        }
        else {
            APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: unknown require %s!", payment_svr_name(svr), logic_require_name(require));
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_RECHARGE_WAY_ERROR);
            return logic_op_exec_result_false;
        }
    }
}

logic_op_exec_result_t
payment_svr_op_recharge_db_update(
    logic_context_t ctx, logic_stack_node_t stack,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, BAG_INFO * bag_info)
{
    PAYMENT_BILL_DATA * bill;
    logic_require_t require;

    /*bill必须在调用前设置完成 */
    bill = logic_data_data(logic_stack_data_find(stack, dr_meta_name(svr->m_meta_bill_data)));
    assert(bill->way == payment_bill_way_in);

    /*发送更新请求 */
    require = logic_require_create(stack, "recharge_update");
    if (require == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge_update:: create logic require fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (payment_svr_db_send_add_money(svr, bag_info, require, req->user_id, &bill->money) != 0) {
        logic_require_free(require);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
payment_svr_op_recharge_on_db_update(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info)
{
    PAYMENT_BILL_DATA * bill_data;
    mongo_cli_result_t update_result;
    int r;

    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    update_result = mongo_cli_result_find(require);
    if (update_result == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge_update:: no update result!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    bill_data = logic_data_data(logic_stack_data_find(stack, dr_meta_name(svr->m_meta_bill_data)));
    assert(bill_data);

    if (mongo_cli_result_n(update_result) == 1) { /*老用户更新成功，查询回数据 */
        if (payment_svr_db_build_balance(svr, bag_info, require, &res->balance) != 0) {
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }

        payment_svr_db_add_bill(svr, bag_info, req->user_id, bill_data, &res->balance);

        return logic_op_exec_result_false;
    }
    else if (mongo_cli_result_n(update_result) == 0) { /*新用户，需要插入记录 */
        logic_require_t insert_require;

        insert_require = logic_require_create(stack, "recharge_insert");
        if (insert_require == NULL) {
            APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge_update: create logic require for insert fail!", payment_svr_name(svr));
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }

        if (payment_svr_db_send_init_money(svr, bag_info, insert_require, req->user_id, &bill_data->money) != 0) {
            logic_require_free(insert_require);
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }

        return logic_op_exec_result_true;
    }
    else {
        APP_CTX_ERROR(
            logic_context_app(ctx), "%s: recharge: on_db_update:: update %d records !!!",
            payment_svr_name(svr), mongo_cli_result_n(update_result));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}

logic_op_exec_result_t
payment_svr_op_recharge_on_db_insert(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info)
{
    PAYMENT_BILL_DATA * bill_data;
    int r;
    uint8_t i;

    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    bill_data = logic_data_data(logic_stack_data_find(stack, dr_meta_name(svr->m_meta_bill_data)));
    assert(bill_data);

    res->balance.count = bag_info->money_type_count;
    for(i = 0; i < bag_info->money_type_count; ++i) {
        res->balance.datas[i].type = PAYMENT_MONEY_TYPE_MIN + i;
        res->balance.datas[i].count = 
            payment_svr_get_count_by_type(&bill_data->money, res->balance.datas[i].type, 0);
    }

    payment_svr_db_add_bill(svr, bag_info, req->user_id, bill_data, &res->balance);

    return logic_op_exec_result_true;
}
