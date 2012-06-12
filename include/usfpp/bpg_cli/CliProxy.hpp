#ifndef USFPP_BPG_CLI_NETCLIENT_H
#define USFPP_BPG_CLI_NETCLIENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/Data.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/bpg_cli/bpg_cli_proxy.h"
#include "System.hpp"

namespace Usf { namespace Bpg {

class CliProxy : public Cpe::Utils::SimulateObject {
public:
    operator bpg_cli_proxy_t() const { return (bpg_cli_proxy_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(bpg_cli_proxy_name(*this)); }
    Gd::App::Application & app(void) { return Gd::App::Application::_cast(bpg_cli_proxy_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(bpg_cli_proxy_app(*this)); }

    PackageManager & pkgManager(void);

    Cpe::Dr::MetaLib const & metaLib(void) const;
    Cpe::Dr::Meta const & meta(const char * metaName) const;

    size_t bufCapacity(void) const { return bpg_cli_proxy_buf_capacity(*this); }
    Cpe::Dr::Data dataBuf(const char * metaName, size_t capacity = 0);
    Cpe::Dr::Data dataBuf(void);
    Usf::Bpg::Package & pkgBuf(void);

    static CliProxy & _cast(bpg_cli_proxy_t cli_proxy);
    static CliProxy & instance(gd_app_context_t app, const char * name);
};

}}

#endif
