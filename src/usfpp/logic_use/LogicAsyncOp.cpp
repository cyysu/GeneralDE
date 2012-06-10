#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_context.h"
#include "gdpp/app/Log.hpp"
#include "cpepp/cfg/Node.hpp"
#include "usf/logic/logic_stack.h"
#include "usf/logic_use/logic_op_async.h"
#include "usfpp/logic/LogicOpContext.hpp"
#include "usfpp/logic/LogicOpStack.hpp"
#include "usfpp/logic_use/LogicAsyncOp.hpp"

namespace Usf { namespace Logic {

LogicAsyncOp::LogicAsyncOp(execute_fun send_fun, execute_fun recv_fun)
    : Logic::LogicOp((execute_fun)&LogicAsyncOp::execute)
    , m_send_fun(send_fun)
    , m_recv_fun(recv_fun)
{
}

logic_op_exec_result_t LogicAsyncOp::send_op_adapter(logic_context_t ctx, logic_stack_node_t stack_node, void * user_data, cfg_t cfg) {
    LogicAsyncOp * op = (LogicAsyncOp*)user_data;
    logic_executor_t executor = logic_stack_node_executor(stack_node);
    try {
        logic_op_exec_result_t rv = (op->*(op->m_send_fun))(*(Logic::LogicOpContext*)ctx, *(Logic::LogicOpStackNode*)stack_node, Cpe::Cfg::Node::_cast(cfg));

        if (logic_context_flag_is_enable(ctx, logic_context_flag_debug)) {
            APP_CTX_INFO(
                logic_context_app(ctx), "execute logic op %s: send complete, errno=%d, state=%d",
                logic_executor_name(executor), logic_context_errno(ctx), logic_context_state(ctx));
        }

        return rv;
    }
    APP_CTX_CATCH_EXCEPTION(logic_context_app(ctx), "%s: send: ", logic_executor_name(executor));
    return logic_op_exec_result_null;
}

logic_op_exec_result_t LogicAsyncOp::recv_op_adapter(logic_context_t ctx, logic_stack_node_t stack_node, void * user_data, cfg_t cfg) {
    LogicAsyncOp * op = (LogicAsyncOp*)user_data;
    logic_executor_t executor = logic_stack_node_executor(stack_node);
    try {
        logic_op_exec_result_t rv = (op->*(op->m_recv_fun))(*(Logic::LogicOpContext*)ctx, *(Logic::LogicOpStackNode*)stack_node, Cpe::Cfg::Node::_cast(cfg));

        if (logic_context_flag_is_enable(ctx, logic_context_flag_debug)) {
            APP_CTX_INFO(
                logic_context_app(ctx), "execute logic op %s: recv complete, errno=%d, state=%d",
                logic_executor_name(executor), logic_context_errno(ctx), logic_context_state(ctx));
        }

        return rv;
    }
    APP_CTX_CATCH_EXCEPTION(logic_context_app(ctx), "%s: recv: ", logic_executor_name(executor));
    return logic_op_exec_result_null;
}

LogicAsyncOp::R LogicAsyncOp::execute(Logic::LogicOpContext & context, Logic::LogicOpStackNode & stackNode, Cpe::Cfg::Node const & cfg) const {
    return logic_op_asnyc_exec(context, stackNode, send_op_adapter, recv_op_adapter, (void*)this, cfg);
}

}}

