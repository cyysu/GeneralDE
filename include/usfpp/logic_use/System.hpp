#ifndef USFPP_LOGIC_USE_SYSTEM_H
#define USFPP_LOGIC_USE_SYSTEM_H
#include "cpepp/dr/System.hpp"
#include "usf/logic_use/logic_use_types.h"
#include "usfpp/logic/System.hpp"

namespace Usf { namespace Logic {

class LogicAsyncOp;

template<typename ListT, typename EleT>
class LogicOpDynList;

template<typename DataT, int count = Cpe::Dr::MetaTraits<DataT>::dyn_count>
class LogicOpDynData;

}}

#endif
