#include <cassert>
#include "cpepp/cfg/Node.hpp"
#include "cpepp/tl/Manager.hpp"
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic/LogicOpDef.hpp"
#include "usfpp/logic_use/LogicOpDynDataT.hpp"
#include "CenterSvrLogicOp.hpp"

namespace Svr { namespace Center {

class CenterSvr_Svr_Query : public LogicOp {
public:
    static cpe_hash_string_t NAME;

    CenterSvr_Svr_Query(
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

USFPP_LOGICOP_DEF_BASE(CenterSvr_Svr_Query);

}}
