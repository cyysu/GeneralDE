#include "cpepp/cfg/Node.hpp"
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic/LogicOpDef.hpp"
#include "usfpp/logic_use/LogicOpDynDataT.hpp"
#include "ChatSvrLogicOp.hpp"
#include "ChanelManager.hpp"
#include "Chanel.hpp"

namespace Svr { namespace Chat {

class ChatSvr_Op_QueryMsg : public LogicOp {
public:
    static cpe_hash_string_t NAME;

    ChatSvr_Op_QueryMsg(
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
        SVR_CHAT_REQ_QUERY_MSG const & req = context.data<SVR_CHAT_REQ_QUERY_MSG>();

        ChanelManager & chanelMgr = ChanelManager::instance(context.app());

        uint32_t require_count = req.require_count;
        if (require_count > 128) require_count = 128;

        Usf::Logic::LogicOpDynDataT<SVR_CHAT_RES_QUERY_MSG> result(context, require_count);

        Chanel * chanel = chanelMgr.findChanel(req.chanel_type, req.chanel_id);
        if (chanel == NULL) {
            APP_CTX_INFO(
                context.app(), "%s: query: chanel %d."FMT_UINT64_T" not exist!",
                name(), req.chanel_type, req.chanel_id);
            return true;
        }

        SVR_CHAT_CHANEL_DATA const & chanelData = chanel->data();

        uint32_t r = chanelData.chanel_msg_r;
        while (require_count > 0) {
            if (r == chanelData.chanel_msg_w) break;

            SVR_CHAT_MSG const & msg = chanel->msg(r);

            if (msg.sn > req.after_sn) {
                --require_count;
                result.recordAppend(msg);
            }

            ++r;
            if (r >= chanelData.chanel_msg_capacity) r = 0;
        }

        return true;
    }
};

USFPP_LOGICOP_DEF_BASE(ChatSvr_Op_QueryMsg);

}}
