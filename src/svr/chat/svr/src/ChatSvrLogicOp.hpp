#ifndef THE9_LMSSVR_LOGICOP_H
#define THE9_LMSSVR_LOGICOP_H
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic_use/LogicAsyncOp.hpp"
#include "ChatSvrLogicOpContext.hpp"

namespace Svr { namespace Chat {

class LogicOp : public Usf::Logic::LogicOpDef<LogicOp, LogicOpContext> {
};

class LogicAsyncOp : public Usf::Logic::LogicAsyncOpDef<LogicAsyncOp, LogicOpContext> {
};

}}

#endif
