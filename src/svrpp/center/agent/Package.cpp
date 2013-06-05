#include <stdexcept>
#include "cpe/dr/dr_metalib_manage.h"
#include "gdpp/app/Log.hpp"
#include "svrpp/center/agent/Package.hpp" 
#include "svrpp/center/agent/Agent.hpp" 

namespace Svr { namespace Center {

Gd::App::Application & Package::app(void) {
    return agent().app();
}

Gd::App::Application const & Package::app(void) const {
    return agent().app();
}

void Package::assertMetaEq(LPDRMETA meta) const {
    LPDRMETA pkg_meta = center_agent_pkg_meta(*this);
    if (pkg_meta != meta) {
        char buf[128];
        snprintf(
            buf, sizeof(buf), "Svr::Center::Package::assertMetaEq %s <==> %s mismatch", 
            pkg_meta ? dr_meta_name(pkg_meta) : "???",
            meta ? dr_meta_name(meta) : "???");
        throw ::std::runtime_error(buf);
    }
}

Package & Package::_cast(dp_req_t req) {
    center_agent_pkg_t pkg = center_agent_pkg_from_dp_req(req);
    if (pkg == NULL) {
        throw ::std::runtime_error("Svr::Center::Package::_cast from req fail");
    }

    return *(Package*)pkg;
}

}}
