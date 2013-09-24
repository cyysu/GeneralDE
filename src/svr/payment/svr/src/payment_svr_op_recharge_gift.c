#include <assert.h>
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_context.h"
#include "gd/app/app_log.h"
#include "payment_svr_ops.h"

logic_op_exec_result_t
payment_svr_op_recharge_gift_send(
    logic_context_t ctx, logic_stack_node_t stack,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info)
{
    logic_data_t bill_data;
    PAYMENT_BILL_DATA * bill;

    if (payment_svr_op_validate_money_types(svr, bag_info, &req->data.gift.moneies) != 0) {
        CPE_ERROR(svr->m_em, "%s: gift_send: validate moneies fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_MONEY_TYPE_ERROR);
        return logic_op_exec_result_false;
    }

    bill_data = logic_stack_data_get_or_create(stack, svr->m_meta_bill_data, sizeof(*bill));
    if (bill_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: gift_send:: create bill_data fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    bill = logic_data_data(bill_data);

    bill->way = payment_bill_way_in;
    memcpy(&bill->money, &req->data.gift.moneies, sizeof(bill->money));
    bill->acitvity_id = req->data.gift.acitvity_id;
    bill->gift_id = req->data.gift.gift_id;

    return payment_svr_op_recharge_db_update(ctx, stack, svr, req, bag_info);
}
