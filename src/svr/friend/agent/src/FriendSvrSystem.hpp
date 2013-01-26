#ifndef SVR_FRIEND_AGENT_SYSTEM_H
#define SVR_FRIEND_AGENT_SYSTEM_H
#include "cpe/pal/pal_types.h"
#include "usfpp/logic/System.hpp"
#include "usfpp/logic_use/System.hpp"
#include "usfpp/bpg_rsp/System.hpp"
#include "protocol/svr/friend/svr_friend_pro.h"

namespace Svr { namespace Friend {

class LogicOp;
class LogicAsyncOp;

typedef Usf::Bpg::RspOpContext LogicOpContext;

typedef Usf::Logic::LogicOpData LogicOpData;
using Usf::Logic::LogicOpDynList;
using Usf::Logic::LogicOpRequire;
using Usf::Logic::LogicOpStackNode;

}}

#endif

