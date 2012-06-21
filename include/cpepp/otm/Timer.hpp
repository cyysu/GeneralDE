#ifndef CPEPP_OTM_TIMER_H
#define CPEPP_OTM_TIMER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/otm/otm_timer.h"
#include "System.hpp"
#include "Memo.hpp"

namespace Cpe { namespace Otm {

class Timer : public Cpe::Utils::SimulateObject {
public:
    operator otm_timer_t (void) const { return (otm_timer_t)(this); }
    
    bool autoEnable(void) const { return otm_timer_auto_enable(*this); }
    void setAautoEnable(bool b) const { return otm_timer_set_auto_enable(*this, b ? 1 : 0); }

    void enable(tl_time_t cur_time, otm_memo_t memo, tl_time_t first_exec_span = 0) {
        otm_timer_enable(*this, cur_time, first_exec_span, memo);
    }

    void disable(otm_memo_t memo) {
        otm_timer_disable(*this, memo);
    }
};

}}

#endif

