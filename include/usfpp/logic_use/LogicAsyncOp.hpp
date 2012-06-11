#ifndef USFPP_LOGIC_USE_LOGICOPASYNC_H
#define USFPP_LOGIC_USE_LOGICOPASYNC_H
#include "cpepp/utils/ClassCategory.hpp"
#include "usfpp/logic/LogicOp.hpp"
#include "System.hpp"

namespace Usf { namespace Logic {

class LogicAsyncOp : public Logic::LogicOp {
public:
    LogicAsyncOp(execute_fun send_fun, execute_fun recv_fun);

private:
    R execute(Logic::LogicOpContext & context, Logic::LogicOpStackNode & stackNode, Cpe::Cfg::Node const & cfg) const;

    execute_fun m_send_fun;
    execute_fun m_recv_fun;

    static logic_op_exec_result_t send_op_adapter(logic_context_t ctx, logic_stack_node_t stack_node, void * user_data, cfg_t cfg);
    static logic_op_exec_result_t recv_op_adapter(logic_context_t ctx, logic_stack_node_t stack_node, void * user_data, cfg_t cfg);
};

template<typename OutT, typename ContextT>
class LogicAsyncOpDef : public LogicAsyncOp {
public:
    LogicAsyncOpDef()
        : LogicAsyncOp(
            (execute_fun)&LogicAsyncOpDef::send,
            (execute_fun)&LogicAsyncOpDef::recv)
    {}

    virtual R send(
        ContextT & context,
        Logic::LogicOpStackNode & stackNode,
        Cpe::Cfg::Node const & cfg) const = 0;

    virtual R recv(
        ContextT & context,
        Logic::LogicOpStackNode & stackNode,
        Cpe::Cfg::Node const & cfg) const = 0;
};

}}

#endif
