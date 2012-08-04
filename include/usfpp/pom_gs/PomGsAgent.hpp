#ifndef USFPP_POM_GS_AGENT_H
#define USFPP_POM_GS_AGENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/dr/dr_data.h"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/pom_gs/pom_gs_agent.h"
#include "System.hpp"

namespace Usf { namespace Pom {

class PomGsAgent : public Cpe::Utils::SimulateObject {
public:
    operator pom_gs_agent_t () const { return (pom_gs_agent_t)this; }

    const char * name(void) const { return pom_gs_agent_name(*this); }
    cpe_hash_string_t name_hs(void) const { return pom_gs_agent_name_hs(*this); }

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(pom_gs_agent_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(pom_gs_agent_app(*this)); }

    void insert(
        pom_grp_obj_mgr_t obj_mgr,
        pom_grp_obj_t obj,
        logic_require_t require);

    void insert(
        LPDRMETA meta, const void * data, size_t capacity,
        logic_require_t require);

    static PomGsAgent & _cast(pom_gs_agent_t agent);
    static PomGsAgent & instance(gd_app_context_t app, const char * name);
};

}}

#endif
