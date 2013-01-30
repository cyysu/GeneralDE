#include <cassert>
#include "cpe/pal/pal_stdio.h"
#include "cpepp/cfg/Node.hpp"
#include "cpepp/tl/Manager.hpp"
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "usfpp/logic/LogicOpStack.hpp"
#include "usfpp/logic/LogicOpDef.hpp"
#include "usfpp/mongo_driver/Package.hpp"
#include "usfpp/mongo_cli/CliProxy.hpp"
#include "usfpp/mongo_cli/Result.hpp"
#include "protocol/svr/account/svr_account_internal.h"
#include "AccountSvrLogicOp.hpp"
#include "AccountSvrLogic.hpp"

namespace Svr { namespace Account {

class AccountSvr_Op_Bind : public LogicAsyncOp {
public:
    static cpe_hash_string_t NAME;

    AccountSvr_Op_Bind(
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
        updateAccount(context, stackNode);

        return true;
    }

    virtual R recv(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
		if(require.name() =="insertAccount" ) {
            return onInsertAccount(context, stackNode, require, args);
		}
		else if(require.name() =="updateAccount" ) {
            return onUpdateAccount(context, stackNode, require, args);
		}
        else {
            APP_CTX_ERROR(context.app(), "%s: unknown requilre %s", name(), require.name().c_str());
            return false;
		}
    }

private:
    void updateAccount(LogicOpContext & context, Usf::Logic::LogicOpStackNode & stackNode) const {
        AccountSvrLogic & logic = AccountSvrLogic::instance(context.app());

        SVR_ACCOUNT_REQ_BIND const & req = context.data<SVR_ACCOUNT_REQ_BIND>();

        Usf::Mongo::CliProxy & db = logic.db();

        Usf::Mongo::Package & pkg = db.pkgBuf();
        pkg.init();

        pkg.setCollection("Accounts");
        pkg.setOp(mongo_db_op_insert);
        pkg.docOpen();
        pkg.appendInt64("_id", (int64_t)req.gid);

        pkg.appendObjectStart("$set");
        pkg.appendString(makeColumnName(req.account_type), req.account);
        pkg.appendObjectFinish();

        pkg.docClose();

        db.send(stackNode.createRequire("updateAccount"), pkg);
    }

    bool onUpdateAccount(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
        if (require.error() != 0) {
            APP_CTX_ERROR(context.app(), "%s: requilre %s: error, err=%d", name(), require.name().c_str(), require.error());
            return false;
        }

        /*默认创建者就是会长 */
        // SVR_ACCOUNT_REQ_BIND const & req = context.data<SVR_ACCOUNT_REQ_BIND>();
        // SVR_ACCOUNT_I_ACCOUNT const & guild = context.data<SVR_ACCOUNT_I_ACCOUNT>();

        // SVR_ACCOUNT_REQ_JOIN & joinReq = context.checkCreateData<SVR_ACCOUNT_REQ_JOIN>();
        // joinReq.member_gid = req.creator_gid;
        // joinReq.guild_gid = guild._id;
        // joinReq.role_count = 1;
        // joinReq.roles[0] = 1;

        return true;
    }

    void insertAccount(LogicOpContext & context, Usf::Logic::LogicOpStackNode & stackNode) const {
        SVR_ACCOUNT_REQ_BIND const & req = context.data<SVR_ACCOUNT_REQ_BIND>();

        AccountSvrLogic & logic = AccountSvrLogic::instance(context.app());

        Usf::Mongo::CliProxy & db = logic.db();

        Usf::Mongo::Package & pkg = db.pkgBuf();
        pkg.init();

        pkg.setCollection("Accounts");
        pkg.setOp(mongo_db_op_insert);

        pkg.docOpen();
        pkg.appendInt64("_id", req.gid);
        
        pkg.docClose();

        db.send(stackNode.createRequire("insertAccount"), pkg);
    }

    bool onInsertAccount(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
        if (require.error() != 0) {
            APP_CTX_ERROR(context.app(), "%s: requilre %s: error, err=%d", name(), require.name().c_str(), require.error());
            return false;
        }

        Usf::Mongo::Result & result = Usf::Mongo::Result::get(require);

        if (result.n() == 0) {
            insertAccount(context, stackNode);
            return true;
        }
        else if (result.n() == 1) {
            return true;
        }
        else {
            APP_CTX_ERROR(context.app(), "%s: update count %d error!", name(), result.n());
            return false;
        }

        return true;
    }

private:
    const char * makeColumnName(uint8_t account_type) const {
        snprintf(m_buf, sizeof(m_buf), "a_%d", account_type);
        return m_buf;
    }

    mutable char m_buf[64];
};

USFPP_LOGICOP_DEF_BASE(AccountSvr_Op_Bind);

}}
