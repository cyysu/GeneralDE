#ifndef USFPP_LOGIC_STACK_H
#define USFPP_LOGIC_STACK_H
#include "cpepp/utils/ClassCategory.hpp"
#include "usf/logic/logic_stack.h"
#include "System.hpp"

namespace Usf { namespace Logic {

class LogicOpStackNode : public Cpe::Utils::SimulateObject {
public:
    operator logic_stack_node_t () const { return (logic_stack_node_t)this; }

    LogicOpData * tryGetData(void) { return (LogicOpData *)logic_stack_node_data(*this); }
    LogicOpData const * tryGetData(void) const { return (LogicOpData *)logic_stack_node_data(*this); }

    LogicOpData & checkCreateData(LPDRMETA meta, size_t capacity = 0);
};

}}

#endif
