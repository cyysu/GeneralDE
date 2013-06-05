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

class AccountSvr_Op_FindByLogicId : public LogicAsyncOp {
public:
    static cpe_hash_string_t NAME;

    AccountSvr_Op_FindByLogicId(
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
        SVR_ACCOUNT_REQ_FIND_BY_LOGIC_ID const & req = context.data<SVR_ACCOUNT_REQ_FIND_BY_LOGIC_ID>();

        AccountSvrLogic & logic = AccountSvrLogic::instance(context.app());

        Usf::Logic::LogicOpDynDataT<SVR_ACCOUNT_BINDING_LIST> result(context);

        for(uint16_t i = 0; i < req.count; ++i) {
            SVR_ACCOUNT_LOGIC_ID const & logic_id = req.ids[i];

            queryAccount(context, stackNode, logic_id, result.recordCount(), logic);

            SVR_ACCOUNT_BINDING & r = result.recordAppend();
            r.account_type = logic_id.account_type;
            strncpy(r.account, logic_id.account, sizeof(r.account));
        }

        return true;
    }

    virtual R recv(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args) const
    {
        int index = -1;
        int r = sscanf(require.name(), "queryAccount%d", &index);
        if (r == (int)strlen(require.name())) {
            return onQueryAccount(context, stackNode, require, args, index);
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
        SVR_ACCOUNT_LOGIC_ID const & logic_id,
        uint16_t index, 
        AccountSvrLogic & logic) const
    {
        Usf::Mongo::CliProxy & db = logic.db();

        Usf::Mongo::Package & pkg = db.pkgBuf();
        pkg.init();

        pkg.setCollection("Accounts");
        pkg.setOp(mongo_db_op_query);
        pkg.docOpen();
        pkg.appendString(makeColumnName(logic_id.account_type), logic_id.account);
        pkg.docClose();

        
        db.send(stackNode.createRequire("queryAccount"), pkg);
    }

    bool onQueryAccount(
        LogicOpContext & context,
        Usf::Logic::LogicOpStackNode & stackNode,
        Usf::Logic::LogicOpRequire & require,
        Cpe::Cfg::Node const & args,
        int index) const
    {
        if (require.error() != 0) {
            APP_CTX_ERROR(context.app(), "%s: requilre %s: error, err=%d", name(), require.name().c_str(), require.error());
            return false;
        }

        Usf::Logic::LogicOpDynDataT<SVR_ACCOUNT_I_ACCOUNT_LIST> query_result(context);
        if (query_result.recordCount() == 0) return true;

        if (query_result.recordCount() != 1) {
            APP_CTX_ERROR(
                context.app(), "%s: requilre %s: query resout count %d error",
                name(), require.name().c_str(), (int)query_result.recordCount());
            return false;
        }

        Usf::Logic::LogicOpDynDataT<SVR_ACCOUNT_BINDING_LIST> result(context);

        if (index < 0 || ((size_t)index) >= result.recordCount()) {
            APP_CTX_ERROR(
                context.app(), "%s: requilre %s: index %d error, count=%d",
                name(), require.name().c_str(), index, (int)result.recordCount());
            return false;
        }

        result.record(index).gid = query_result.record(0)._id;

        return true;
    }

private:
    void appendBinding(SVR_ACCOUNT_BINDING & r, uint64_t gid, int type, const char * account) const {
        r.gid = gid;
        r.account_type = type;
        strncpy(r.account, account, sizeof(r.account));
    }

    const char * makeColumnName(uint8_t account_type) const {
        snprintf(m_buf, sizeof(m_buf), "a_%d", account_type);
        return m_buf;
    }

    mutable char m_buf[64];
};

USFPP_LOGICOP_DEF_BASE(AccountSvr_Op_FindByLogicId);

}}
