#ifndef CPEPP_TL_MANAGER_H
#define CPEPP_TL_MANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/tl/tl_manage.h"
#include "System.hpp"

namespace Cpe { namespace Tl {

class Manager : public Cpe::Utils::SimulateObject {
public:
    operator tl_manage_t (void) const { return (tl_manage_t)(this); }

    tl_manage_state_t state(void) const { return tl_manage_state(*this); }
    void pause(void) { tl_manage_pause(*this); }
    void resume(void) { tl_manage_resume(*this); }

    tl_time_t curTime(void) const { return tl_manage_time(*this); }

    int tick(int count = -1) { return tl_manage_tick(*this, count); }
};

}}

#endif
