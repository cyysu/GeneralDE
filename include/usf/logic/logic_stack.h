#ifndef USF_LOGIC_STACK_H
#define USF_LOGIC_STACK_H
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_executor_t logic_stack_node_executor(logic_stack_node_t stack);
logic_data_t logic_stack_node_data(logic_stack_node_t stack);
logic_data_t logic_stack_node_data_check_or_create(logic_stack_node_t stack, LPDRMETA meta, size_t capacity);
void logic_stack_node_data_clear(logic_stack_node_t stack);

#ifdef __cplusplus
}
#endif

#endif
