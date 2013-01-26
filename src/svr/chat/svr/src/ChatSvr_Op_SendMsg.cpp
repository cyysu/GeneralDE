#include <cassert>
#include "cpepp/cfg/Node.hpp"
#include "cpepp/tl/Manager.hpp"
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic/LogicOpDef.hpp"
#include "usfpp/logic_use/LogicOpDynDataT.hpp"
#include "ChatSvrLogicOp.hpp"
#include "ChanelManager.hpp"
#include "Chanel.hpp"

namespace Svr { namespace Chat {

class ChatSvr_Op_SendMsg : public LogicOp {
public:
    static cpe_hash_string_t NAME;

    ChatSvr_Op_SendMsg(
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
        SVR_CHAT_REQ_SEND_MSG const & req = context.data<SVR_CHAT_REQ_SEND_MSG>();

        ChanelManager & chanelMgr = ChanelManager::instance(context.app());

        Chanel * chanel = chanelMgr.findChanel(req.chanel_type, req.chanel_id);
        if (chanel == NULL) {
            chanel = chanelMgr.createChanel(req.chanel_type, req.chanel_id);
            if (chanel == NULL) {
                APP_CTX_ERROR(
                    context.app(), "%s: create chanel %d."FMT_UINT64_T" fail!",
                    name(), req.chanel_type, req.chanel_id);
                return false;
            }
        }

        assert(chanel);

        SVR_CHAT_MSG & msg = chanel->appendMsg();

        msg.send_time = context.app().tlManager().curTimeSec();
        msg.sender_id = req.sender_id;
        strncpy(msg.sender_name, req.sender_name, sizeof(msg.sender_name));
        strncpy(msg.msg, req.msg, sizeof(msg.msg));

        return true;
    }
};

USFPP_LOGICOP_DEF_BASE(ChatSvr_Op_SendMsg);

}}
