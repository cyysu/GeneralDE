#include <assert.h>
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "friend_svr_ops.h"
#include "protocol/svr/friend/svr_friend_pro.h"
#include "protocol/svr/friend/svr_friend_internal.h"

logic_op_exec_result_t
friend_svr_op_add_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    friend_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_FRIEND_REQ_ADD const * req;
    int rv;
    char buf[svr->m_record_size];

    req_data = logic_context_data_find(ctx, "svr_friend_req_add");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: get request fail!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    bzero(buf, sizeof(buf));

    rv = dr_pbuf_read(
        buf + svr->m_record_data_start_pos,
        sizeof(buf) - svr->m_record_data_start_pos,
        req->data,
        req->data_len,
        svr->m_data_meta,
        svr->m_em);
    if (rv <= 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: decode data error, rv=%d!", friend_svr_name(svr), rv);
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (dr_entry_set_from_uint64(buf + svr->m_record_uid_start_pos, req->user_id, svr->m_record_uid_entry, svr->m_em) != 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: set uid error!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (friend_svr_set_record_id(svr, buf) != 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: set _id error!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    require = logic_require_create(stack, "insert");
    if (require == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: create logic require fail!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (friend_svr_db_send_insert(svr, require, buf) != 0) {
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
friend_svr_op_add_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    friend_svr_t svr = user_data;

    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) != logic_require_state_error) {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: add: db request state error, state=%s!",
                friend_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
        else if (logic_require_error(require) == mongo_data_error_duplicate_key) {
            if (svr->m_debug) {
                APP_CTX_INFO(
                    logic_context_app(ctx), "%s: add: friend already exist!",
                    friend_svr_name(svr));
            }

            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_ALREADY_EXIST);

            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: add: db request error, errno=%d!",
                friend_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_DB);
            return logic_op_exec_result_false;
        }
    }

    return logic_op_exec_result_true;
}
