#include <stdexcept>
#include "cpepp/otm/Manager.hpp"

namespace Cpe { namespace Otm {

void ManagerBase::registerTimer(
        otm_timer_id_t id,
        const char * name,
        void * realResponser, void * fun,
        tl_time_span_t span
#ifdef _MSC_VER
        , void * useResponser
#endif
        )
{
    
}

void ManagerBase::unregisterTimer(otm_timer_id_t id) {
}

void ManagerBase::unregisterTimer(void *processor) {
}

ManagerBase & ManagerBase::_cast(otm_manage_t otm) {
    if (otm == NULL) {
        throw ::std::runtime_error("otm is NULL!"); 
    }

    return *reinterpret_cast<ManagerBase*>(otm);
}

}}
