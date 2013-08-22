#include <stdexcept>
#include "cpe/dr/dr_metalib_manage.h"
#include "cpepp/dp/Request.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "svrpp/set/logic/SendPoint.hpp" 

namespace Svr { namespace Set {

void SendPoint::sendData(
    uint16_t to_svr_type, uint16_t to_svr_id,
    LPDRMETA meta, void const * data, size_t size,
    logic_require_t require)
{
    if (set_logic_sp_send_req_data(*this, to_svr_type, to_svr_id, meta, data, size, require) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "center_logic_sp %s: send data %s(len=%d) fail!", name(), dr_meta_name(meta), (int)size);
    }
}

void SendPoint::sendCmd(
    uint16_t to_svr_type, uint16_t to_svr_id,
    uint32_t cmd, logic_require_t require)
{
    if (set_logic_sp_send_req_cmd(*this, to_svr_type, to_svr_id, cmd, require) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "center_logic_sp %s: send cmd %d fail!", name(), (int)cmd);
    }
}

void SendPoint::sendPkg(dp_req_t pkg, logic_require_t require) {
    if (set_logic_sp_send_pkg(*this, pkg, require) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "center_logic_sp %s: send pkg fail!", name());
    }
}

SendPoint & SendPoint::instance(gd_app_context_t app, const char * name) {
    set_logic_sp_t sp = set_logic_sp_find_nc(app, name);
    if (sp == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "center_logic_sp %s not exist!", name);
    }

    return *(SendPoint*)sp;
}

}}
