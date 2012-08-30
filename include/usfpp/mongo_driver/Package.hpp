#ifndef USFPP_MONGO_PACKAGE_H
#define USFPP_MONGO_PACKAGE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "usf/mongo_driver/mongo_pkg.h"
#include "System.hpp"

namespace Usf { namespace Mongo {

class Agent : public Cpe::Utils::SimulateObject {
public:
    operator mongo_agent_t() const { return (mongo_agent_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(mongo_agent_name(*this)); }
    Gd::App::Application & app(void) { return Gd::App::Application::_cast(mongo_agent_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(mongo_agent_app(*this)); }

    void send(mongo_pkg_t pkg, logic_require_t require);

    static Agent & _cast(mongo_agent_t agent);
    static Agent & instance(gd_app_context_t app, const char * name);
};

}}

#endif
