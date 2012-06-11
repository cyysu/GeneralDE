#ifndef USFPP_LOGIC_STACK_H
#define USFPP_LOGIC_STACK_H
#include "cpepp/utils/ClassCategory.hpp"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_stack.h"
#include "System.hpp"

namespace Usf { namespace Logic {

class LogicOpStackNode : public Cpe::Utils::SimulateObject {
public:
    operator logic_stack_node_t () const { return (logic_stack_node_t)this; }

    LogicOpExecutor & executor(void) { return *(LogicOpExecutor*)logic_stack_node_executor(*this); }
    LogicOpExecutor const & executor(void) const { return *(LogicOpExecutor*)logic_stack_node_executor(*this); }

    LogicOpContext & context(void) { return *(LogicOpContext*)logic_stack_node_context(*this); }
    LogicOpContext const & context(void) const { return *(LogicOpContext*)logic_stack_node_context(*this); }

    LogicOpData & data(const char * name);
    LogicOpData const & data(const char * name) const;

    LogicOpData * findData(const char * name) { return (LogicOpData *)logic_stack_data_find(*this, name); }
    LogicOpData const * findData(const char * name) const { return (LogicOpData *)logic_stack_data_find(*this, name); }

    LogicOpData & checkCreateData(LPDRMETA meta, size_t capacity = 0);
};

}}

#endif
