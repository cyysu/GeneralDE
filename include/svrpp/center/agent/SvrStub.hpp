#ifndef SVRPP_CENTER_AGENT_SVRSTUB_H
#define SVRPP_CENTER_AGENT_SVRSTUB_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "svr/center/agent/center_agent_svr_stub.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Center {

class SvrStub : public Cpe::Utils::SimulateObject {
public:
    operator center_agent_svr_stub_t() const { return (center_agent_svr_stub_t)this; }

    static SvrStub & get(Agent & agent, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
