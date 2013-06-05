#include <cassert>
#include "cpepp/utils/ErrorCollector.hpp"
#include "cpepp/dr/Data.hpp"
#include "cpepp/dr/Entry.hpp"
#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/MetaLib.hpp"
#include "gdpp/app/Log.hpp"
#include "usfpp/bpg_pkg/Package.hpp"
#include "usfpp/bpg_pkg/PackageManager.hpp"

namespace Usf { namespace Bpg {

Gd::App::Application & Package::app(void) {
    return mgr().app();
}

Gd::App::Application const & Package::app(void) const {
    return mgr().app();
}

Cpe::Dr::MetaLib const &
Package::dataMetaLib(void) const {
    LPDRMETALIB metalib = bpg_pkg_data_meta_lib(*this);
    if (metalib == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s: Usf::Bpg::Package::dataMetaLib: not data meta lib!",
            mgr().name().c_str());
    }

    return *(Cpe::Dr::MetaLib const *)metalib;
}

PackageAppendInfo const & Package::appendInfoAt(int32_t pos) const {
    bpg_pkg_append_info_t appendInfo = bpg_pkg_append_info_at(*this, pos);
    if (appendInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "%s: Usf::Bpg::Package::appendInfoAt: pos %d overflow!",
            mgr().name().c_str(), pos);
    }
    return *(PackageAppendInfo const *)appendInfo;
}

Cpe::Dr::Meta const &
Package::mainDataMeta(void) const {
    Cpe::Utils::ErrorCollector em;

    LPDRMETA meta = bpg_pkg_main_data_meta(*this, em);
    if (meta == NULL) {
        em.checkThrowWithMsg< ::std::runtime_error>();
    }

    return Cpe::Dr::Meta::_cast(meta);
}

void Package::setCmdAndData(Cpe::Dr::ConstData const & data) {
    setCmd(mgr().cmdFromMetaName(data.meta().name()));
    setMainData(data.data(), data.capacity());
}

void Package::setCmdAndData(Cpe::Dr::Data const & data) {
    setCmd(mgr().cmdFromMetaName(data.meta().name()));
    setMainData(data.data(), data.capacity());
}

void Package::setCmdAndData(Cpe::Dr::ConstData const & data, size_t size) {
    setCmd(mgr().cmdFromMetaName(data.meta().name()));
    setMainData(data.data(), size);
}

void Package::setCmdAndData(int cmd, const void * data, size_t data_size) {
    setCmd(cmd);
    setMainData(data, data_size);
}

void Package::setCmdAndData(const char * meta_name, const void * data, size_t data_size) {
    setCmd(mgr().cmdFromMetaName(meta_name));
    setMainData(data, data_size);
}
        
void Package::setCmdAndData(LPDRMETA meta, const void * data, size_t data_size) {
    setCmd(mgr().cmdFromMetaName(dr_meta_name(meta)));
    setMainData(data, data_size);
}

void Package::setMainData(void const * data, size_t size) {
    Cpe::Utils::ErrorCollector em;
    if (bpg_pkg_set_main_data(*this, data, size, em) != 0) {
        em.checkThrowWithMsg< ::std::runtime_error>();
    }
}

void Package::mainData(Cpe::Dr::Data & data) {
    size_t size = mainDataSize();
    if (data.capacity() < size) {
        throw ::std::runtime_error("Usf::Bpg::Package::mainData: not enough buf!");
    }

    memcpy(data.data(), mainData(), mainDataSize());
    data.setCapacity(mainDataSize());
    data.setMeta(mainDataMeta());
}

void Package::addAppendData(const char * metaName, void const * data, size_t size) {
    addAppendData(dataMetaLib().meta(metaName), data, size);
}

void Package::addAppendData(int metaid, void const * data, size_t size) {
    addAppendData(dataMetaLib().meta(metaid), data, size);
}

void Package::addAppendData(LPDRMETA meta, void const * data, size_t size) {
    Cpe::Utils::ErrorCollector em;

    if (bpg_pkg_add_append_data(*this, meta, data, size, em)) {
        em.checkThrowWithMsg< ::std::runtime_error>();
    }
}

void Package::appendData(int metaId, void * buf, size_t capacity, size_t * size) const {
    appendData(dataMetaLib().meta(metaId), buf, capacity, size);
}

void Package::appendData(const char * metaName, void * buf, size_t capacity, size_t * size) const {
    appendData(dataMetaLib().meta(metaName), buf, capacity, size);
}

void Package::appendData(LPDRMETA meta, void * buf, size_t capacity, size_t * r_size) const {
    bpg_pkg_append_info_t appendInfo = 0;
    for(int32_t i = 0; i < bpg_pkg_append_info_count(*this); ++i) {
        appendInfo = bpg_pkg_append_info_at(*this, i);
        if ((int)bpg_pkg_append_info_id(appendInfo) == (int)dr_meta_id(meta)) {
            size_t size = bpg_pkg_append_info_size(appendInfo);
            if (size > capacity) {
                APP_CTX_THROW_EXCEPTION(
                    app(),
                    ::std::runtime_error,
                    "%s: Usf::Bpg::Package::appendData: no enough buf to get data meta %s(%d), capacity=%d, size=%d!",
                    mgr().name().c_str(), dr_meta_name(meta), dr_meta_id(meta), (int)capacity, (int)size);
                return;
            }

            memcpy(buf, bpg_pkg_append_data(*this, appendInfo), size);

            if (r_size) *r_size = size;

            return;
        }
    }

    APP_CTX_THROW_EXCEPTION(
        app(),
        ::std::runtime_error,
        "%s: Usf::Bpg::Package::appendData: no append info of meta %s(%d)!",
        mgr().name().c_str(), dr_meta_name(meta), dr_meta_id(meta));
}

bool Package::tryGetAppendData(int metaId, void * buf, size_t capacity, size_t * size) const {
    LPDRMETALIB metalib = bpg_pkg_data_meta_lib(*this);
    if (metalib == NULL) return false;

    LPDRMETA meta = dr_lib_find_meta_by_id(metalib, metaId);
    if (meta == NULL) return false;

    return tryGetAppendData(meta, buf, capacity, size);
}

bool Package::tryGetAppendData(const char * metaName, void * buf, size_t capacity, size_t * size) const {
    LPDRMETALIB metalib = bpg_pkg_data_meta_lib(*this);
    if (metalib == NULL) return false;

    LPDRMETA meta = dr_lib_find_meta_by_name(metalib, metaName);
    if (meta == NULL) return false;

    return tryGetAppendData(meta, buf, capacity, size);
}

bool Package::tryGetAppendData(LPDRMETA meta, void * buf, size_t capacity, size_t * r_size) const {
    bpg_pkg_append_info_t appendInfo = 0;
    for(int32_t i = 0; i < bpg_pkg_append_info_count(*this); ++i) {
        appendInfo = bpg_pkg_append_info_at(*this, i);
        if ((int)bpg_pkg_append_info_id(appendInfo) == (int)dr_meta_id(meta)) {
            size_t size = bpg_pkg_append_info_size(appendInfo);
            if (size > capacity) return false;

            memcpy(buf, bpg_pkg_append_data(*this, appendInfo), size);

            if (r_size) *r_size = size;

            return true;
        }
    }

    return false;
}

Package & Package::_cast(bpg_pkg_t pkg) {
    if (pkg == NULL) {
        throw ::std::runtime_error("Usf::Bpg::Package::_cast: input pkg is NULL!");
    }

    return *(Package*)pkg;
}

Package & Package::_cast(dp_req_t req) {
    if (req == NULL) {
        throw ::std::runtime_error("Usf::Bpg::Package::_cast: input req is NULL!");
    }

    bpg_pkg_t pkg = bpg_pkg_from_dp_req(req);
    if (pkg == NULL) {
        throw ::std::runtime_error("Usf::Bpg::Package::_cast: cast dp_req to pkg fail!");
    }

    return *(Package*)pkg;
}

}}
