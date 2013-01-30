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
#include "protocol/svr/account/svr_account_internal.h"
#include "AccountSvrLogicOp.hpp"
#include "AccountSvrLogic.hpp"

namespace Svr { namespace Account {

class AccountSvr_Op_FindByGid : public LogicAsyncOp {
public:
    static cpe_hash_string_t NAME;

    AccountSvr_Op_FindByGid(
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
        SVR_ACCOUNT_REQ_FIND_BY_GID const & req = context.data<SVR_ACCOUNT_REQ_FIND_BY_GID>();

        AccountSvrLogic & logic = AccountSvrLogic::instance(context.app());

        queryAccount(context, stackNode, req.gid, logic);

        return true;
    }

    virtual R recv(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
		if(require.name() =="queryAccount" ) {
            return onQueryAccount(context, stackNode, require, args);
		}
        else {
            APP_CTX_ERROR(context.app(), "%s: unknown requilre %s", name(), require.name().c_str());
            return false;
		}
    }

private:
    void queryAccount(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        uint64_t gid,
        AccountSvrLogic & logic) const
    {
        Usf::Mongo::CliProxy & db = logic.db();

        Usf::Mongo::Package & pkg = db.pkgBuf();
        pkg.init();

        pkg.setCollection("Accounts");
        pkg.setOp(mongo_db_op_query);
        pkg.docOpen();
        pkg.appendInt64("_id", gid);
        pkg.docClose();

        db.send(stackNode.createRequire("queryAccount"), pkg);
    }

    bool onQueryAccount(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
        if (require.error() != 0) {
            APP_CTX_ERROR(context.app(), "%s: requilre %s: error, err=%d", name(), require.name().c_str(), require.error());
            return false;
        }

        Usf::Logic::LogicOpDynDataT<SVR_ACCOUNT_I_ACCOUNT_LIST> query_result(context);
        Usf::Logic::LogicOpDynDataT<SVR_ACCOUNT_BINDING_LIST> result(context);

        for(size_t i = 0; i < query_result.recordCount(); ++i) {
            SVR_ACCOUNT_I_ACCOUNT const & qr = query_result.record(i);

            if (qr.a_1[0]) appendBinding(result.recordAppend(), qr._id, 1, qr.a_1);
            if (qr.a_2[0]) appendBinding(result.recordAppend(), qr._id, 2, qr.a_2);
            if (qr.a_3[0]) appendBinding(result.recordAppend(), qr._id, 3, qr.a_3);
            if (qr.a_4[0]) appendBinding(result.recordAppend(), qr._id, 4, qr.a_4);
        }

        return true;
    }

private:
    void appendBinding(SVR_ACCOUNT_BINDING & r, uint64_t gid, int type, const char * account) const {
        r.gid = gid;
        r.account_type = type;
        strncpy(r.account, account, sizeof(r.account));
    }

};

USFPP_LOGICOP_DEF_BASE(AccountSvr_Op_FindByGid);

}}
