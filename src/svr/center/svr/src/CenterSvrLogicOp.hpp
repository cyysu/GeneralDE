#ifndef THE9_CENTER_LOGICOP_H
#define THE9_CENTER_LOGICOP_H
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic_use/LogicAsyncOp.hpp"
#include "CenterSvrLogicOpContext.hpp"

namespace Svr { namespace Center {

class LogicOp : public Usf::Logic::LogicOpDef<LogicOp, LogicOpContext> {
};

class LogicAsyncOp : public Usf::Logic::LogicAsyncOpDef<LogicAsyncOp, LogicOpContext> {
};

}}

#endif
