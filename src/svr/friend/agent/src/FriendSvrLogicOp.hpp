#ifndef SVR_FRIEND_AGENT_LOGICOP_H
#define SVR_FRIEND_AGENT_LOGICOP_H
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic_use/LogicAsyncOp.hpp"
#include "FriendSvrLogicOpContext.hpp"

namespace Svr { namespace Friend {

class LogicOp : public Usf::Logic::LogicOpDef<LogicOp, LogicOpContext> {
};

class LogicAsyncOp : public Usf::Logic::LogicAsyncOpDef<LogicAsyncOp, LogicOpContext> {
};

}}

#endif
