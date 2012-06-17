#ifndef CPEPP_OTM_SYSTEM_H
#define CPEPP_OTM_SYSTEM_H
#include "cpe/otm/otm_types.h"

namespace Cpe { namespace Otm {

template<size_t capacity> class MemoBuf;
class Memo;

class Manager;
class TimerProcess;

typedef void (TimerProcessor::*TimerProcessFun)(Memo & memo, tl_time_t cur_exec_time, void * obj_ctx);

}}

#endif
