#ifndef SVRPP_SET_STUB_STUB_H
#define SVRPP_SET_STUB_STUB_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "svr/set/stub/set_svr_stub.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Set {

class Stub : public Cpe::Utils::SimulateObject {
public:
    operator set_svr_stub_t() const { return (set_svr_stub_t)this; }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)set_svr_stub_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application*)set_svr_stub_app(*this); }

    template<typename T>
    void sendData(uint16_t svr_type, uint16_t svr_id, uint16_t sn, T const & data) {
        send(svr_type, svr_id, sn, &data, Cpe::Dr::MetaTraits<T>::data_size(data), Cpe::Dr::MetaTraits<T>::META);
    }

    void sendData(uint16_t svr_type, uint16_t svr_id, uint16_t sn, void const * data, uint16_t data_size, LPDRMETA meta);
    void sendCmd(uint16_t svr_type, uint16_t svr_id, uint16_t sn, uint32_t cmd);
    void sendPkg(uint16_t svr_type, uint16_t svr_id, uint16_t sn, dp_req_t pkg);

    const char * name(void) const { return set_svr_stub_name(*this); }

    static Stub & instance(gd_app_context_t app, const char * name = "set_svr_stub");
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
