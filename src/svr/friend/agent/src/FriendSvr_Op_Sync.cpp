#include <cassert>
#include "cpepp/cfg/Node.hpp"
#include "cpepp/tl/Manager.hpp"
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "usfpp/logic/LogicOpStack.hpp"
#include "usfpp/logic/LogicOpDef.hpp"
#include "usfpp/mongo_driver/Package.hpp"
#include "usfpp/mongo_cli/CliProxy.hpp"
#include "usfpp/mongo_cli/Result.hpp"
#include "FriendSvrLogicOp.hpp"
#include "FriendSvrLogic.hpp"

namespace Svr { namespace Friend {

class FriendSvr_Op_Sync : public LogicAsyncOp {
public:
    static cpe_hash_string_t NAME;

    FriendSvr_Op_Sync(
        Gd::App::Application & app,
        Gd::App::Module & module,
        Cpe::Cfg::Node & moduleCfg)
    {
    }


    virtual R send(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Cpe::Cfg::Node const & args) const
    {
        return true;
    }

    virtual R recv(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
        return true;
    }
};

USFPP_LOGICOP_DEF_BASE(FriendSvr_Op_Sync);

}}
