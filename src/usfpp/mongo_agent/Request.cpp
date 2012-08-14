#include "gdpp/app/Log.hpp"
#include "usfpp/mongo_agent/Request.hpp"
#include "usfpp/mongo_agent/Agent.hpp"

namespace Usf { namespace Mongo {

Request & Request::get(mongo_agent_t agent, const char * name) {
    mongo_request_t requiest = mongo_request_get(agent, name);
    if (requiest == NULL) {
        APP_CTX_THROW_EXCEPTION(
            mongo_agent_app(agent),
            ::std::runtime_error,
            "mongo_request %s not exist in %s!", name, mongo_agent_name(agent));
    }

    return *(Request*)requiest;
}

void Request::addRecord(LPDRMETA meta, const void * data, size_t size) {
    if (mongo_request_add_record(*this, meta, data, size) != 0) {
        mongo_agent_t agent = mongo_request_agent(*this);

        APP_CTX_THROW_EXCEPTION(
            mongo_agent_app(agent),
            ::std::runtime_error,
            "%s: request add data %s(size="FMT_SIZE_T") fail!",
            mongo_agent_name(agent), dr_meta_name(meta), size);
    }
}

}}
