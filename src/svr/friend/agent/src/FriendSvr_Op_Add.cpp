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

class FriendSvr_Op_Add : public LogicAsyncOp {
public:
    static cpe_hash_string_t NAME;

    FriendSvr_Op_Add(
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
        FriendSvrLogic & logic = FriendSvrLogic::instance(context.app());

        SVR_FRIEND_REQ_ADD const  req =  context.data<SVR_FRIEND_REQ_ADD>();

        Usf::Mongo::CliProxy & db = logic.db();

        Usf::Mongo::Package & pkg = db.pkgBuf();
        pkg.init();

        char buf[64];
        snprintf(buf, sizeof(buf), FMT_UINT64_T"-"FMT_UINT64_T, req.gid, req.friend_gid);

        pkg.setCollection("Friends");
        pkg.setOp(mongo_db_op_insert);

        pkg.docOpen();
        pkg.appendString("_id",buf);
        pkg.appendInt64("gid", (int64_t)req.gid);
        pkg.appendInt64("friend_gid", (int64_t)req.friend_gid);
        pkg.docClose();

        db.send(stackNode.createRequire("insertFriend"), pkg);

        return true;
    }

    virtual R recv(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
		if(require.name() =="insertFriend" ) {
            return onInsertFriend(context, stackNode, require, args);
		}
        else {
            APP_CTX_ERROR(context.app(), "%s: unknown require %s", name(), require.name().c_str());
            return false;
		}
    }

    bool onInsertFriend(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
        if (require.error() != 0) {
            APP_CTX_ERROR(context.app(), "%s: require %s: error, err=%d", name(), require.name().c_str(), require.error());
            return false;
        }

        return true;
    }
};

USFPP_LOGICOP_DEF_BASE(FriendSvr_Op_Add);

}}
