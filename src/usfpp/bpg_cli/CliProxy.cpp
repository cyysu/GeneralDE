#include "cpepp/dr/MetaLib.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Log.hpp"
#include "usfpp/bpg_pkg/Package.hpp"
#include "usfpp/bpg_cli/CliProxy.hpp"

namespace Usf { namespace Bpg {

PackageManager &
CliProxy::pkgManager(void) {
    bpg_pkg_manage_t pkg_manage = bpg_cli_proxy_pkg_manage(*this);
    if (pkg_manage == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "CliProxy %s have no pkg_manage!", name().c_str());
    }

    return *(PackageManager*)pkg_manage;
}

CliProxy & CliProxy::_cast(bpg_cli_proxy_t cli_proxy) {
    if (cli_proxy == NULL) {
        throw ::std::runtime_error("Usf::Bpg::CliProxy::_cast: input cli_proxy is NULL!");
    }

    return *(CliProxy*)cli_proxy;
}

CliProxy & CliProxy::instance(gd_app_context_t app, const char * name) {
    bpg_cli_proxy_t cli_proxy = bpg_cli_proxy_find_nc(app, name);
    if (cli_proxy == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "bpg_cli_proxy %s not exist!", name ? name : "default");
    }

    return *(CliProxy*)cli_proxy;
}

Cpe::Dr::MetaLib const & CliProxy::metaLib(void) const {
    LPDRMETALIB metalib = bpg_cli_proxy_metalib(*this);
    if (metalib == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: meta lib not exist!", name().c_str());
    }

    return Cpe::Dr::MetaLib::_cast(metalib);
}

Cpe::Dr::Meta const & CliProxy::meta(const char * metaName) const {
    LPDRMETA meta = bpg_cli_proxy_meta(*this, metaName);
    if (meta == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: meta %s not exist!", name().c_str(), metaName);
    }

    return Cpe::Dr::Meta::_cast(meta);
}

Usf::Bpg::Package & CliProxy::pkgBuf(void) {
    bpg_pkg_t pkg_buf = bpg_cli_proxy_pkg_buf(*this);
    if (pkg_buf == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: pkg-buf is NULL!", name().c_str());
    }

    return Usf::Bpg::Package::_cast(pkg_buf);
}

Cpe::Dr::Data CliProxy::dataBuf(const char * metaName, size_t capacity) {
    void * buf = bpg_cli_proxy_data_buf(*this);
    if (buf == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: data-buf is NULL!", name().c_str());
    }

    size_t buf_capacity = bpg_cli_proxy_buf_capacity(*this);

    Cpe::Dr::Meta const & m = meta(metaName);

    if (capacity == 0) capacity = m.size();

    if (capacity > buf_capacity) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: require size overflow! require %d, but only %d",
            name().c_str(), (int)capacity, (int)buf_capacity);
    }

    return Cpe::Dr::Data(buf, meta(metaName), capacity);
}

Cpe::Dr::Data CliProxy::dataBuf(void) {
    void * buf = bpg_cli_proxy_data_buf(*this);
    if (buf == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: data-buf is NULL!", name().c_str());
    }

    return Cpe::Dr::Data(buf, bpg_cli_proxy_buf_capacity(*this));
}

}}
