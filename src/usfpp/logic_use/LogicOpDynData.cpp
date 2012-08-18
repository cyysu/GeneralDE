#include "gdpp/app/Log.hpp"
#include "usf/logic/logic_manage.h"
#include "usfpp/logic_use/LogicOpDynData.hpp"

namespace Usf { namespace Logic {

void * LogicOpDynData::record(size_t i) {
    void * r = logic_data_record_at(*this, i);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(*this)),
            ::std::runtime_error,
            "Tsf4wg::TCaplus::LogicOpDynData::recrd: get record at %d fail", (int)i);
    }

    return r;
}

void const * LogicOpDynData::record(size_t i) const {
    void * r = logic_data_record_at(*this, i);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(*this)),
            ::std::runtime_error,
            "Tsf4wg::TCaplus::LogicOpDynData::recrd: get record at %d fail", (int)i);
    }

    return r;
}

void * LogicOpDynData::recordAppend(void) {
    void * r = logic_data_record_append(*this);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(*this)),
            ::std::runtime_error,
            "Tsf4wg::TCaplus::LogicOpDynData::append: fail");
    }

    return r;
}

}}
