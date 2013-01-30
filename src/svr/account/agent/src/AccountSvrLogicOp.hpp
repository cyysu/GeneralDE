#ifndef SVR_ACCOUNT_AGENT_LOGICOP_H
#define SVR_ACCOUNT_AGENT_LOGICOP_H
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic_use/LogicAsyncOp.hpp"
#include "AccountSvrLogicOpContext.hpp"

namespace Svr { namespace Account {

class LogicOp : public Usf::Logic::LogicOpDef<LogicOp, LogicOpContext> {
};

class LogicAsyncOp : public Usf::Logic::LogicAsyncOpDef<LogicAsyncOp, LogicOpContext> {
};

}}

#endif
