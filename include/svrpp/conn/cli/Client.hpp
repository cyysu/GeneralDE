#ifndef SVRPP_CONN_CLI_CLIENT_H
#define SVRPP_CONN_CLI_CLIENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "svr/conn/cli/conn_cli.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Conn {

class Client : public Cpe::Utils::SimulateObject {
public:
    operator conn_cli_t() const { return (conn_cli_t)this; }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)conn_cli_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application*)conn_cli_app(*this); }

    static Client & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
