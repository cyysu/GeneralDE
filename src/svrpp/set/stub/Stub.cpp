#include "gdpp/app/Log.hpp"
#include "svrpp/set/stub/Stub.hpp"

namespace Svr { namespace Set {

Stub & Stub::instance(gd_app_context_t app, const char * name) {
    set_svr_stub_t stub = set_svr_stub_find_nc(app, name);
    if (stub == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "set_svr_stub %s not exist!", name);
    }

    return *(Stub*)stub;
}


}}
