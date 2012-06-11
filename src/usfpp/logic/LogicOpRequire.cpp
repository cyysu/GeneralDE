#include "cpe/cfg/cfg_manage.h"
#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/MetaLib.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Random.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "usfpp/logic/LogicOpContext.hpp"

namespace Usf { namespace Logic {

LogicOpData & LogicOpRequire::data(const char * name) {
    LogicOpData * r = findData(name);
    if (r == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s not exist in require!",
            name);
    }
    return *r;
}

LogicOpData const & LogicOpRequire::data(const char * name) const {
    LogicOpData const * r = findData(name);
    if (r == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s not exist in require!",
            name);
    }
    return *r;
}

LogicOpData &
LogicOpRequire::checkCreateData(LPDRMETA meta, size_t capacity) {
    logic_data_t data = logic_require_data_get_or_create(*this, meta, capacity);
    if (data == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s create in require fail!",
            dr_meta_name(meta));
    }
    return *(LogicOpData*)data;
}

}}
