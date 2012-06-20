#ifndef CPEPP_OTM_MEMO_H
#define CPEPP_OTM_MEMO_H
#include "cpepp/utils/ClassCategory.hpp"
#include "System.hpp"

namespace Cpe { namespace Otm {

class Memo : public Cpe::Utils::SimulateObject {
public:
    operator otm_memo_t (void) const { return (otm_memo_t)(this); }
};

}}

#endif
