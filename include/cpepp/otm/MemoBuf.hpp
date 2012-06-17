#ifndef CPEPP_OTM_MEMOBUF_H
#define CPEPP_OTM_MEMOBUF_H
#include "System.hpp"

namespace Cpe { namespace Otm {

template<size_t capacity>
class MemoBuf {

private:
    struct otm_memo m_memos[capacity];
};

}}

#endif
