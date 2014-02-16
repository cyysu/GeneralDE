#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "gd/utils/id_generator.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "usf/mongo_use/id_generator.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "account_svr_ops.h"
#include "protocol/svr/account/svr_account_pro.h"
#include "protocol/svr/account/svr_account_internal.h"

logic_op_exec_result_t
account_svr_op_create_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_ACCOUNT_REQ_LOGIN const * req;
    logic_data_t res_data;
    SVR_ACCOUNT_RES_LOGIN * res;

    req_data = logic_context_data_find(ctx, "svr_account_req_create");
    if (req_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: create: get request fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_create, 0);
    if (res_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: create: create result!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);

    if (gd_id_generator_generate(&res->account_id, (gd_id_generator_t)svr->m_id_generator, "account_id") != 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: create: gen account id fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    require = logic_require_create(stack, "create_query");
    if (require == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: create: create logic require fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    
    if (account_svr_db_send_insert(svr, require, res->account_id, req->passwd, &req->logic_id) != 0) {
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        logic_require_free(require);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
account_svr_op_create_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;

    /*检查数据库返回结果 */
    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            APP_CTX_ERROR(
                svr->m_app, "%s: create: db request error, errno=%d!",
                account_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                svr->m_app, "%s: create: db request state error, state=%s!",
                account_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_DB);
            return logic_op_exec_result_false;
        }
    }

    return logic_op_exec_result_true;
}
