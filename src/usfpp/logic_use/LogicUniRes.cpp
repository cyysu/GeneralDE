#include "gdpp/app/Log.hpp"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_require.h"
#include "usfpp/logic_use/LogicUniRes.hpp"

namespace Usf { namespace Logic {

LogicOpDynData & LogicUniRes::get_at(logic_require_t require) {
    LogicOpDynData * r = find_at(require);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_require_mgr(require)),
            ::std::runtime_error,
            "Tsf4wg::TCaplus::LogicUniRes::get_at: get result fail");
    }

    return *r;
}

void LogicUniRes::init_at(logic_require_t require, LPDRMETA meta, size_t record_capacity) {
    if (logic_uni_res_init(require, meta, record_capacity) != 0) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_require_mgr(require)),
            ::std::runtime_error,
            "Tsf4wg::TCaplus::LogicUniRes::get_at: get result fail");
    }
}

}}
