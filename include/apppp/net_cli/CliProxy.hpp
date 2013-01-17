#ifndef APPPP_NET_CLI_NETCLIENT_H
#define APPPP_NET_CLI_NETCLIENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/Data.hpp"
#include "gdpp/app/Application.hpp"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace App { namespace Net {

class CliProxy : public Cpe::Utils::SimulateObject {
public:
    operator app_net_cli_t() const { return (app_net_cli_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(app_net_cli_name(*this)); }
    Gd::App::Application & app(void) { return Gd::App::Application::_cast(app_net_cli_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(app_net_cli_app(*this)); }


    static CliProxy & _cast(app_net_cli_t cli_proxy);
    static CliProxy & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
