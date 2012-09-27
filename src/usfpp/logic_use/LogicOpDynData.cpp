#include "gdpp/app/Log.hpp"
#include "usf/logic/logic_manage.h"
#include "usfpp/logic_use/LogicOpDynData.hpp"

namespace Usf { namespace Logic {

void * LogicOpDynData::record(size_t i) {
    validate_data();

    void * r = logic_data_record_at(m_data, i);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(m_data)),
            ::std::runtime_error,
            "Tsf4wg::TCaplus::LogicOpDynData::recrd: get record at %d fail", (int)i);
    }

    return r;
}

void const * LogicOpDynData::record(size_t i) const {
    validate_data();

    void * r = logic_data_record_at(m_data, i);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(m_data)),
            ::std::runtime_error,
            "Tsf4wg::TCaplus::LogicOpDynData::recrd: get record at %d fail", (int)i);
    }

    return r;
}

void * LogicOpDynData::recordAppend(void) {
    validate_data();

    if (isDynamic()) {
        size_t count = logic_data_record_count(m_data);
        size_t capacity = logic_data_record_capacity(m_data);

        if (count >= capacity) {
            size_t new_capacity  = capacity < 16 ? 16 : capacity * 2;
            gd_app_context_t app = logic_manage_app(logic_data_mgr(m_data));

            m_data = logic_data_record_reserve(m_data, new_capacity);
            if (m_data == NULL) {
                APP_CTX_THROW_EXCEPTION(
                    app,
                    ::std::runtime_error,
                    "Tsf4wg::TCaplus::LogicOpDynData::append: reserve to %d fail", (int)new_capacity);
            }
        }
    }

    void * r = logic_data_record_append(m_data);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_manage_app(logic_data_mgr(m_data)),
            ::std::runtime_error,
            "Tsf4wg::TCaplus::LogicOpDynData::append: fail");
    }

    return r;
}

void LogicOpDynData::validate_data(void) const {
    if (m_data == NULL) {
        throw ::std::runtime_error("LogicOpDynData: data is invalid!"); 
    }
}

}}
