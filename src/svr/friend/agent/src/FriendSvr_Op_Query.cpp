#include <cassert>
#include "cpepp/cfg/Node.hpp"
#include "cpepp/tl/Manager.hpp"
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "usfpp/logic/LogicOpStack.hpp"
#include "usfpp/logic/LogicOpDef.hpp"
#include "usfpp/logic_use/LogicOpDynDataT.hpp"
#include "usfpp/mongo_driver/Package.hpp"
#include "usfpp/mongo_cli/CliProxy.hpp"
#include "usfpp/mongo_cli/Result.hpp"
#include "protocol/svr/friend/svr_friend_internal.h"
#include "FriendSvrLogicOp.hpp"
#include "FriendSvrLogic.hpp"

namespace Svr { namespace Friend {

class FriendSvr_Op_Query : public LogicAsyncOp {
public:
    static cpe_hash_string_t NAME;

    FriendSvr_Op_Query(
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

        SVR_FRIEND_REQ_QUERY const  req =  context.data<SVR_FRIEND_REQ_QUERY>();

        Usf::Mongo::CliProxy & db = logic.db();

        Usf::Mongo::Package & pkg = db.pkgBuf();
        pkg.init();

        pkg.setCollection("Friends");
        pkg.setOp(mongo_db_op_query);

        pkg.docOpen();
        pkg.appendObjectStart("$query");
        pkg.appendInt64("gid", (int64_t)req.gid);
        pkg.appendObjectFinish();
        pkg.docClose();

        db.send(stackNode.createRequire("queryFriend"), pkg, Cpe::Dr::MetaTraits<SVR_FRIEND_I_FRIEND_LIST>::META);

        return true;
    }

    virtual R recv(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
        if(require.name() =="queryFriend" ) {
            return onQueryFriend(context, stackNode, require, args);
		}
        else {
            APP_CTX_ERROR(context.app(), "%s: unknown require %s", name(), require.name().c_str());
            return false;
		}
    }

    bool onQueryFriend(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
        if (require.error() != 0) {
            APP_CTX_ERROR(context.app(), "%s: require %s: error, err=%d", name(), require.name().c_str(), require.error());
            return false;
        }

        Usf::Logic::LogicOpDynDataT<SVR_FRIEND_I_FRIEND_LIST> friendsInDB(require); 
        Usf::Logic::LogicOpDynDataT<SVR_FRIEND_RES_QUERY> result(context);

        for(uint32_t i = 0; i < friendsInDB.recordCount(); ++i) {
            result.recordAppend(friendsInDB.record(i).friend_gid);
        }

        return true;
    }
};

USFPP_LOGICOP_DEF_BASE(FriendSvr_Op_Query);

}}
