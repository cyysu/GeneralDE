#include <stdexcept>
#include "gdpp/app/Log.hpp"
#include "svrpp/conn/cli/Client.hpp" 

namespace Svr { namespace Conn {

Client & Client::instance(gd_app_context_t app, const char * name) {
    conn_cli_t cli = conn_cli_find_nc(app, name);
    if (cli == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "conn_cli %s not exist!", name);
    }

    return *(Client*)cli;
}

}}
