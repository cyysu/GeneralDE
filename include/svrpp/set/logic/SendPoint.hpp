#ifndef SVRPP_SET_LOGIC_SENDPOINT_H
#define SVRPP_SET_LOGIC_SENDPOINT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "cpepp/dp/System.hpp"
#include "svr/set/logic/set_logic_sp.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Set {

class SendPoint : public Cpe::Utils::SimulateObject {
public:
    operator set_logic_sp_t() const { return (set_logic_sp_t)this; }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)set_logic_sp_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application*)set_logic_sp_app(*this); }

    Stub & stub(void) { return *(Stub*)set_logic_sp_stub(*this); }
    Stub const & stub(void) const { return *(Stub const *)set_logic_sp_stub(*this); }

    const char * name(void) const { return set_logic_sp_name(*this); }

    void sendData(
        uint16_t to_svr_type, uint16_t to_svr_id,
        LPDRMETA meta, void const * data, size_t size,
        logic_require_t require = NULL);

    void sendCmd(
        uint16_t to_svr_type, uint16_t to_svr_id,
        uint32_t cmd,
        logic_require_t require = NULL);

    void sendPkg(dp_req_t pkg, logic_require_t require = NULL);

    template<typename T>
    void sendData(uint16_t to_svr_type, uint16_t to_svr_id, T const & data, logic_require_t require = NULL) {
        sendData(to_svr_type, to_svr_id, Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data), require);
    }

    static SendPoint & instance(gd_app_context_t app, const char * name = NULL);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
