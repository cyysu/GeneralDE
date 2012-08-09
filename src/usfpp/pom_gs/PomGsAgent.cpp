#include "gd/app/app_context.h"
#include "gdpp/app/Log.hpp"
#include "usfpp/pom_gs/PomGsAgent.hpp"

namespace Usf { namespace Pom {

PomGsAgent & PomGsAgent::_cast(pom_gs_agent_t agent) {
    if (agent == NULL) {
        throw ::std::runtime_error("Usf::Bpg::PomGsAgent::_cast: input agent is NULL!");
    }

    return *(PomGsAgent*)agent;
}

PomGsAgent & PomGsAgent::instance(gd_app_context_t app, const char * name) {
    pom_gs_agent_t agent = pom_gs_agent_find_nc(app, name);
    if (agent == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "pom_gs_agent %s not exist!", name ? name : "default");
    }

    return *(PomGsAgent*)agent;
}

void PomGsAgent::insert(pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, logic_require_t require) {
    if (pom_gs_agent_obj_insert(*this, obj_mgr, obj, require) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "pom_gs_agent %s insert obj (obj) fail!", name());
    }
}

void PomGsAgent::insert(pom_gs_pkg_t pkg, logic_require_t require) {
    if (pom_gs_agent_data_insert(*this, pkg, require) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "pom_gs_agent %s insert obj (pkg) fail!", name());
    }
}

}}
