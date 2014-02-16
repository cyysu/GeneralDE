#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "account_svr_ops.h"
#include "protocol/svr/account/svr_account_pro.h"
#include "protocol/svr/account/svr_account_internal.h"

logic_op_exec_result_t
account_svr_op_login_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_ACCOUNT_REQ_LOGIN const * req;

    req_data = logic_context_data_find(ctx, "svr_account_req_login");
    if (req_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: login: get request fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    require = logic_require_create(stack, "login_query");
    if (require == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: login: create logic require fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (account_svr_db_send_query_by_logic_id(svr, require, &req->logic_id, svr->m_meta_record_basic_list) != 0) {
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        logic_require_free(require);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
account_svr_op_login_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    logic_data_t db_res;
    SVR_ACCOUNT_BASIC_LIST * account_list;
    SVR_ACCOUNT_BASIC * account;
    logic_data_t res_data;
    SVR_ACCOUNT_RES_LOGIN * res;
    SVR_ACCOUNT_REQ_LOGIN const * req;
    uint16_t from_svr_type;
    uint16_t from_svr_id;
    int r;

    /*检查数据库返回结果 */
    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            APP_CTX_ERROR(
                svr->m_app, "%s: login: db request error, errno=%d!",
                account_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                svr->m_app, "%s: login: db request state error, state=%s!",
                account_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_DB);
            return logic_op_exec_result_false;
        }
    }

    /*获取数据库返回结果 */
    db_res = logic_require_data_find(require, dr_meta_name(svr->m_meta_record_basic_list));
    if (db_res == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: login: find db result fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    account_list = logic_data_data(db_res);

    req = logic_data_data(logic_context_data_find(ctx, "svr_account_req_login"));

    /*检查记录数 */
    if (account_list->count == 0) {
        mem_buffer_clear_data(&svr->m_dump_buffer);

        APP_CTX_ERROR(
            svr->m_app, "%s: login: account %s not exist!", account_svr_name(svr),
            dr_json_dump(&svr->m_dump_buffer, &req->logic_id, sizeof(req->logic_id), svr->m_meta_logic_id));

        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_NOT_EXIST);

        return logic_op_exec_result_false;
    }
    else if (account_list->count > 1) {
        mem_buffer_clear_data(&svr->m_dump_buffer);

        APP_CTX_ERROR(
            svr->m_app, "%s: login: find duplicate accounts\n%s!", account_svr_name(svr),
            dr_json_dump(&svr->m_dump_buffer, account_list, logic_data_capacity(db_res), svr->m_meta_record_basic_list));

        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);

        return logic_op_exec_result_false;
    }

    /*获取登陆请求 */
    account = &account_list->data[0];

    /*检查密码是否符合 */
    r = account_svr_validate_passwd(svr, account, &req->logic_id, req->passwd);
    if (r != 0) {
        APP_CTX_ERROR(svr->m_app, "%s: login: validate passwd fail, rv=%d!", account_svr_name(svr), r);
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    /*获取连接信息 */
    if (set_logic_rsp_context_get_conn_info(ctx, &from_svr_type, &from_svr_id) != 0) {
        APP_CTX_ERROR(svr->m_app, "%s: login: get_carry_info fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    /*如果从连接服务过来的请求，需要将账号绑定到连接上去 */
    if (account_svr_is_conn_svr(svr, from_svr_type)) {
        if (account_svr_conn_bind_account(svr, ctx, from_svr_id, from_svr_type, account->_id) != 0) {
            APP_CTX_ERROR(svr->m_app, "%s: login: set_carry_info not exist!", account_svr_name(svr));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
    }

    /*构造响应 */
    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_login, 0);
    if (res_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: login: create response fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);

    res->account_id = account->_id;

    return logic_op_exec_result_true;
}
