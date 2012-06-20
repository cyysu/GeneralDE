#ifndef CPEPP_OTM_TIMER_H
#define CPEPP_OTM_TIMER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/otm/otm_timer.h"
#include "System.hpp"

namespace Cpe { namespace Otm {

class Timer : public Cpe::Utils::SimulateObject {
public:
    operator otm_timer_t (void) const { return (otm_timer_t)(this); }
};

}}

#endif

