#ifndef SVRPP_CENTER_AGENT_SVR_PKG_H
#define SVRPP_CENTER_AGENT_SVR_PKG_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "svr/center/agent/center_agent_pkg.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Center {

class Package : public Cpe::Utils::SimulateObject {
public:
    operator center_agent_pkg_t() const { return (center_agent_pkg_t)this; }
    operator dp_req_t() const { return center_agent_pkg_to_dp_req(*this); }

    Agent & agent(void) { return *(Agent*)center_agent_pkg_agent(*this); }
    Agent const & agent(void) const { return *(Agent const *)center_agent_pkg_agent(*this); }

    Gd::App::Application & app(void);
    Gd::App::Application const & app(void) const;

    void init(void) { center_agent_pkg_init(*this); }

    center_agent_pkg_category_t category(void) const { return center_agent_pkg_category(*this); }
    void setCategory(center_agent_pkg_category_t c) { center_agent_pkg_set_category(*this, c); }

    uint32_t sn(void) const { return center_agent_pkg_sn(*this); }
    void setSn(uint32_t sn) { center_agent_pkg_set_sn(*this, sn); }

    uint16_t fromSvrType(void) const { return center_agent_pkg_from_svr_type(*this); }
    uint16_t fromSvrId(void) const { return center_agent_pkg_from_svr_id(*this); }
    void setFromSvr(uint16_t from_svr_type, uint16_t from_svr_id) { center_agent_pkg_set_from_svr(*this, from_svr_type, from_svr_id); }

    uint16_t toSvrType(void) const { return center_agent_pkg_to_svr_type(*this); }
    uint16_t toSvrId(void) const { return center_agent_pkg_to_svr_id(*this); }
    void setToSvr(uint16_t to_svr_type, uint16_t to_svr_id) { center_agent_pkg_set_to_svr(*this, to_svr_type, to_svr_id); }

    /*main data read*/
    Cpe::Dr::Meta const & meta(void) const { return *(Cpe::Dr::Meta const *)center_agent_pkg_meta(*this); }
    void setMeta(LPDRMETA meta) {  center_agent_pkg_set_meta(*this, meta); }
    void assertMetaEq(LPDRMETA meta) const;

    size_t size(void) const { return center_agent_pkg_size(*this); }
    void setSize(size_t size) { center_agent_pkg_set_size(*this, size); }

    size_t capacity(void) const { return center_agent_pkg_capacity(*this); }

    void const * data(void) const { return center_agent_pkg_data(*this); }
    void * data(void) { return center_agent_pkg_data(*this); }

    template<typename T>
    T & setAs(void) { setMeta(Cpe::Dr::MetaTraits<T>::META); return *(T*)data(); }

    template<typename T>
    T & as(void) { assertMetaEq(Cpe::Dr::MetaTraits<T>::META); return *(T*)data(); }
    template<typename T>
    T const & as(void) const { assertMetaEq(Cpe::Dr::MetaTraits<T>::META); return *(T*)data(); }

    /*other op*/
    const char * dump_data(mem_buffer_t buffer) const { return center_agent_pkg_dump(*this, buffer); }

    static Package & _cast(center_agent_pkg_t pkg) { return *(Package *)(pkg); }
    static Package & _cast(dp_req_t req);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
