#include "cpepp/cfg/Node.hpp"
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic/LogicOpDef.hpp"
#include "usfpp/logic_use/LogicOpDynDataT.hpp"
#include "ChatSvrLogicOp.hpp"
#include "ChatSvrChanelManager.hpp"

namespace Svr { namespace Chat {

class ChatSvr_Op_Chanel_Create : public LogicOp {
public:
    static cpe_hash_string_t NAME;

    ChatSvr_Op_Chanel_Create(
        Gd::App::Application & app,
        Gd::App::Module & module,
        Cpe::Cfg::Node & moduleCfg)
    {
    }

    virtual R execute(
        LogicOpContext & context, 
        Usf::Logic::LogicOpStackNode & stackNode,
        Cpe::Cfg::Node const & args) const
    {
        return true;
    }
};

USFPP_LOGICOP_DEF_BASE(ChatSvr_Op_Chanel_Create);

}}
