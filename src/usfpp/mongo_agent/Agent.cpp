#include "gdpp/app/Log.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "usfpp/mongo_agent/Agent.hpp"

namespace Usf { namespace Mongo {

Agent & Agent::_cast(mongo_agent_t agent) {
    if (agent == NULL) {
        throw ::std::runtime_error("Tsf4wg::TCaplus::Agent::_cast: input agent is NULL!");
    }

    return *(Agent*)agent;
}

Agent & Agent::instance(gd_app_context_t app, const char * name) {
    mongo_agent_t agent = mongo_agent_find_nc(app, name);
    if (agent == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "mongo_agent %s not exist!", name ? name : "default");
    }

    return *(Agent*)agent;
}

void Agent::send(mongo_request_t request, logic_require_t require) {
    if (mongo_agent_send_request(*this, request, require) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s: send request fail!", name().c_str());
    }
}

}}
