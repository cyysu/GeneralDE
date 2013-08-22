#include <stdexcept>
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "svrpp/conn/net_cli/NetClient.hpp" 

namespace Svr { namespace Conn {

NetClient & NetClient::instance(gd_app_context_t app, const char * name) {
    conn_net_cli_t cli = conn_net_cli_find_nc(app, name);
    if (cli == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "conn_net_cli %s not exist!", name);
    }

    return *(NetClient*)cli;
}

void NetClient::send(uint16_t to_svr, uint32_t sn, LPDRMETA meta, void const * data, uint16_t data_len) {
    if (conn_net_cli_send(*this, to_svr, sn, meta, data, data_len) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "conn_net_cli %s send %s to svr %d fail!", name(), dr_meta_name(meta), to_svr);
    }
}

}}
