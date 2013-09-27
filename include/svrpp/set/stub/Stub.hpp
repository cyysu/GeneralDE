#ifndef SVRPP_SET_STUB_STUB_H
#define SVRPP_SET_STUB_STUB_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
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

    SvrType const & svrType(void) const { return *(SvrType const *)set_svr_stub_svr_type(*this); };

    uint16_t svrId(void) const { return set_svr_stub_svr_id(*this); }

    SvrType const & svrType(uint16_t svr_type_id) const;
    SvrType const & svrType(const char * svr_type_name) const;

    SvrType const * findSvrType(uint16_t svr_type_id) const {
        return (SvrType const *)set_svr_svr_info_find(*this, svr_type_id);
    }

    SvrType const * findSvrType(const char * svr_type_name) const {
        return (SvrType const *)set_svr_svr_info_find_by_name(*this, svr_type_name);
    }

    template<typename T>
    void sendReqData(uint16_t svr_type, uint16_t svr_id, uint16_t sn, T const & data) {
        sendReqData(svr_type, svr_id, sn, &data, Cpe::Dr::MetaTraits<T>::data_size(data), Cpe::Dr::MetaTraits<T>::META);
    }

    template<typename T, typename T2>
    void sendReqData(uint16_t svr_type, uint16_t svr_id, uint16_t sn, T const & data, T2 const & carry_data) {
        sendReqData(
            svr_type, svr_id, sn,
            &data, Cpe::Dr::MetaTraits<T>::data_size(data), Cpe::Dr::MetaTraits<T>::META,
            &carry_data, Cpe::Dr::MetaTraits<T2>::data_size(carry_data));
    }

    template<typename T>
    void sendResponseData(uint16_t svr_type, uint16_t svr_id, uint16_t sn, T const & data) {
        sendResponseData(svr_type, svr_id, sn, &data, Cpe::Dr::MetaTraits<T>::data_size(data), Cpe::Dr::MetaTraits<T>::META);
    }

    template<typename T, typename T2>
    void sendResponseData(uint16_t svr_type, uint16_t svr_id, uint16_t sn, T const & data, T2 const & carry_data) {
        sendResponseData(
            svr_type, svr_id, sn,
            &data, Cpe::Dr::MetaTraits<T>::data_size(data), Cpe::Dr::MetaTraits<T>::META,
            &carry_data, Cpe::Dr::MetaTraits<T2>::data_size(carry_data));
    }

    template<typename T>
    void sendNotifyData(uint16_t svr_type, uint16_t svr_id, T const & data) {
        sendNotifyData(svr_type, svr_id, &data, Cpe::Dr::MetaTraits<T>::data_size(data), Cpe::Dr::MetaTraits<T>::META);
    }

    template<typename T, typename T2>
    void sendNotifyData(uint16_t svr_type, uint16_t svr_id, T const & data, T2 const & carry_data) {
        sendNotifyData(
            svr_type, svr_id,
            &data, Cpe::Dr::MetaTraits<T>::data_size(data), Cpe::Dr::MetaTraits<T>::META,
            &carry_data, Cpe::Dr::MetaTraits<T2>::data_size(carry_data));
    }

    void sendReqData(
        uint16_t svr_type, uint16_t svr_id, uint16_t sn, 
        void const * data, uint16_t data_size, LPDRMETA meta,
        void const * carry_data = NULL, size_t carry_data_size = 0);

    void sendReqCmd(
        uint16_t svr_type, uint16_t svr_id, uint16_t sn,
        uint32_t cmd,
        void const * carry_data = NULL, size_t carry_data_size = 0);

    void sendResponseData(
        uint16_t svr_type, uint16_t svr_id, uint16_t sn,
        void const * data, uint16_t data_size, LPDRMETA meta,
        void const * carry_data = NULL, size_t carry_data_size = 0);

    void sendResponseCmd(
        uint16_t svr_type, uint16_t svr_id, uint16_t sn,
        uint32_t cmd,
        void const * carry_data = NULL, size_t carry_data_size = 0);

    void sendNotifyData(
        uint16_t svr_type, uint16_t svr_id,
        void const * data, uint16_t data_size, LPDRMETA meta,
        void const * carry_data = NULL, size_t carry_data_size = 0);

    void sendNotifyCmd(
        uint16_t svr_type, uint16_t svr_id,
        uint32_t cmd,
        void const * carry_data = NULL, size_t carry_data_size = 0);

    void sendPkg(uint16_t svr_type, uint16_t svr_id, uint16_t sn, dp_req_t pkg);

    const char * name(void) const { return set_svr_stub_name(*this); }

    static Stub & instance(gd_app_context_t app, const char * name = "set_svr_stub");
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
