#ifndef USF_LOGIC_USE_TYPES_H
#define USF_LOGIC_USE_TYPES_H
#include "usf/logic/logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef logic_op_exec_result_t (*logic_op_recv_fun_t) (logic_context_t ctx, logic_stack_node_t stack_noe, logic_require_t require, void * user_data, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif
