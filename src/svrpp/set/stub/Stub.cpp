#include "cpe/dr/dr_metalib_manage.h"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "svrpp/set/stub/Stub.hpp"

namespace Svr { namespace Set {

Stub & Stub::instance(gd_app_context_t app, const char * name) {
    set_svr_stub_t stub = set_svr_stub_find_nc(app, name);
    if (stub == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "set_svr_stub %s not exist!", name);
    }

    return *(Stub*)stub;
}

SvrType const & Stub::svrType(uint16_t svr_type_id) const {
    SvrType const * r = findSvrType(svr_type_id);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "Stub %s: svr type %d not exist!", name(), svr_type_id);
    }

    return *r;
}

SvrType const & Stub::svrType(const char * svr_type_name) const {
    SvrType const * r = findSvrType(svr_type_name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "Stub %s: svr type %s not exist!", name(), svr_type_name);
    }

    return *r;
}

void Stub::sendReqData(
    uint16_t svr_type, uint16_t svr_id, uint16_t sn,
    void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_req_data(*this, svr_type, svr_id, sn, data, data_size, meta, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send %s(size=%d) to %d.%d fail!",
            name(), dr_meta_name(meta), data_size, svr_type, svr_id);
    }
}

void Stub::sendReqCmd(
    uint16_t svr_type, uint16_t svr_id, uint16_t sn,
    uint32_t cmd,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_req_cmd(*this, svr_type, svr_id, sn, cmd, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send cmd %d to %d.%d fail!",
            name(), cmd, svr_type, svr_id);
    }
}

void Stub::sendNotifyData(
    uint16_t svr_type, uint16_t svr_id, 
    void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_notify_data(*this, svr_type, svr_id, 0, data, data_size, meta, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send %s(size=%d) to %d.%d fail!",
            name(), dr_meta_name(meta), data_size, svr_type, svr_id);
    }
}

void Stub::sendNotifyCmd(
    uint16_t svr_type, uint16_t svr_id,
    uint32_t cmd,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_notify_cmd(*this, svr_type, svr_id, 0, cmd, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send cmd %d to %d.%d fail!",
            name(), cmd, svr_type, svr_id);
    }
}

void Stub::sendResponseData(
    uint16_t svr_type, uint16_t svr_id, uint16_t sn,
    void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_response_data(*this, svr_type, svr_id, sn, data, data_size, meta, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send %s(size=%d) to %d.%d fail!",
            name(), dr_meta_name(meta), data_size, svr_type, svr_id);
    }
}

void Stub::sendResponseCmd(
    uint16_t svr_type, uint16_t svr_id, uint16_t sn,
    uint32_t cmd,
    void const * carry_data, size_t carry_data_size)
{
    if (set_svr_stub_send_response_cmd(*this, svr_type, svr_id, sn, cmd, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "Stub %s: send cmd %d to %d.%d fail!",
            name(), cmd, svr_type, svr_id);
    }
}

}}
