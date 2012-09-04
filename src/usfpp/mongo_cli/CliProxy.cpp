#include "gdpp/app/Log.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "usfpp/mongo_driver/Package.hpp"
#include "usfpp/mongo_cli/CliProxy.hpp"

namespace Usf { namespace Mongo {

CliProxy & CliProxy::_cast(mongo_cli_proxy_t agent) {
    if (agent == NULL) {
        throw ::std::runtime_error("Usf::Mongo::CliProxy::_cast: input agent is NULL!");
    }

    return *(CliProxy*)agent;
}

CliProxy & CliProxy::instance(gd_app_context_t app, const char * name) {
    mongo_cli_proxy_t proxy = mongo_cli_proxy_find_nc(app, name);
    if (proxy == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "mongo_agent %s not exist!", name ? name : "default");
    }

    return *(CliProxy*)proxy;
}

void CliProxy::send(logic_require_t require, mongo_pkg_t pkg) {
    if (mongo_cli_proxy_send(*this, pkg, require) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s: send request fail!", name().c_str());
    }
}

void CliProxy::send(mongo_pkg_t pkg) {
    if (mongo_cli_proxy_send(*this, pkg, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s: send request fail!", name().c_str());
    }
}

Package & CliProxy::pkgBuf(void) {
    mongo_pkg_t pkg_buf = mongo_cli_proxy_pkg_buf(*this);
    if (pkg_buf == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: pkg-buf is NULL!", name().c_str());
    }

    return Package::_cast(pkg_buf);
}

}}
