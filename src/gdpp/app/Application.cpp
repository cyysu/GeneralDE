#include <limits>
#include "gd/app/app_basic.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"

namespace Gd { namespace App {

Application &
Application::instance(void) {
    gd_app_context_t ins = gd_app_ins();
    if (ins == NULL) {
        throw ::std::runtime_error("Application have not been created!");
    }

    return *(Application*)ins;
}

Application &
Application::_cast(gd_app_context_t ctx) {
    if (ctx == NULL) {
        throw ::std::runtime_error("cast to Application fail: input ctx is NULL!");
    }
    return *(Application*)ctx;
}

Cpe::Tl::Manager & Application::tlManager(const char * name) {
    Cpe::Tl::Manager * r = findTlManager(name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(*this, ::std::runtime_error, "TlManager %s not exist!", name);
    }

    return *r;
}

Cpe::Tl::Manager const & Application::tlManager(const char * name) const {
    Cpe::Tl::Manager const * r = findTlManager(name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(*this, ::std::runtime_error, "TlManager %s not exist!", name);
    }

    return *r;
}

}}
