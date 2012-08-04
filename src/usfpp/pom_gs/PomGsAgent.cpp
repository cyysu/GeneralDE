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
            "bpg_agent %s not exist!", name ? name : "default");
    }

    return *(PomGsAgent*)agent;
}

}}
