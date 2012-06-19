#ifndef CPEPP_OTM_SYSTEM_H
#define CPEPP_OTM_SYSTEM_H
#include "cpe/tl/tl_types.h"
#include "cpe/otm/otm_types.h"

namespace Cpe { namespace Otm {

template<size_t capacity> class MemoBuf;
class Memo;

class ManagerBase;

template<typename ContextT>
class Manager;

template<typename ContextT>
class TimerProcessor;

}}

#endif
