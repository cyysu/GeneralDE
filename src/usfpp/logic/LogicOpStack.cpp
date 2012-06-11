#include "cpe/cfg/cfg_manage.h"
#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/MetaLib.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Random.hpp"
#include "usfpp/logic/LogicOpStack.hpp"
#include "usfpp/logic/LogicOpContext.hpp"

namespace Usf { namespace Logic {

LogicOpData & LogicOpStackNode::data(const char * name) {
    LogicOpData * r = findData(name);
    if (r == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s not exist in stack!",
            name);
    }
    return *r;
}

LogicOpData const & LogicOpStackNode::data(const char * name) const {
    LogicOpData const * r = findData(name);
    if (r == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s not exist in stack!",
            name);
    }
    return *r;
}

LogicOpData &
LogicOpStackNode::checkCreateData(LPDRMETA meta, size_t capacity) {
    logic_data_t data = logic_stack_data_get_or_create(*this, meta, capacity);
    if (data == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s create in stack fail!",
            dr_meta_name(meta));
    }
    return *(LogicOpData*)data;
}

}}
