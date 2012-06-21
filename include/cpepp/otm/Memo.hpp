#ifndef CPEPP_OTM_MEMO_H
#define CPEPP_OTM_MEMO_H
#include "cpepp/utils/ClassCategory.hpp"
#include "System.hpp"

namespace Cpe { namespace Otm {

class Memo : public Cpe::Utils::SimulateObject {
public:
    operator otm_memo_t (void) const { return (otm_memo_t)(this); }

    tl_time_t lastActionTime(void) const { return ((otm_memo_t)this)->m_last_action_time; }
    tl_time_t nextActionTime(void) const { return ((otm_memo_t)this)->m_next_action_time; }
    void setNextActionTime(tl_time_t t) { ((otm_memo_t)this)->m_next_action_time = t; }
};

}}

#endif
