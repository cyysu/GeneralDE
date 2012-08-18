#ifndef USFPP_BPG_USE_SENDPOINT_H
#define USFPP_BPG_USE_NETCLIENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/Data.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/bpg_use/bpg_use_sp.h"
#include "System.hpp"

namespace Usf { namespace Bpg {

class SendPoint : public Cpe::Utils::Noncopyable {
public:
    SendPoint(gd_app_context_t app, cfg_t cfg);
    ~SendPoint();

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(bpg_use_sp_name(m_sp)); }
    Gd::App::Application & app(void) { return Gd::App::Application::_cast(bpg_use_sp_app(m_sp)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(bpg_use_sp_app(m_sp)); }

    PackageManager const & pkgManager(void) const;

    Cpe::Dr::MetaLib const & metaLib(void) const;
    Cpe::Dr::Meta const & meta(const char * metaName) const;

    size_t dataBufCapacity(void) const { return bpg_use_sp_buf_capacity(m_sp); }
    Cpe::Dr::Data dataBuf(const char * metaName, size_t capacity = 0);
    Cpe::Dr::Data dataBuf(void);
    Usf::Bpg::Package & pkgBuf(void);

    void send(Usf::Bpg::Package & pkg);
    void send(Cpe::Dr::Data const & data);
    void send(const char * metaName, void const * data, size_t size);
    void send(LPDRMETA meta, void const * data, size_t size);

    template<typename T>
    void send(const char * metaName, T const & data) {
        send(metaName, &data, sizeof(data));
    }

    template<typename T>
    void send(LPDRMETA meta, T const & data) {
        send(meta, &data, sizeof(data));
    }

private:
    bpg_use_sp_t m_sp;
};

}}

#endif
