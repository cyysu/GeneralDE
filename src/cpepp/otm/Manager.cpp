#include "cpepp/otm/Manager.hpp"

namespace Cpe { namespace Otm {

void Manager::registerTimer(
        otm_timer_id_t id,
        const char * name,
        TimerProcessor& realResponser, TimerProcessFun fun
        tl_time_span_t span,
#ifdef _MSC_VER
        , TimerProcessor& useResponser
#endif
        )
{
    
}

void Manager::unregisterTimer(otm_timer_id_t id) {
}

void Manager::unregisterTimer(TimerProcessor const & processor) {
}

}}
