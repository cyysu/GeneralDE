#ifndef USF_LOGIC_OP_EXECUTOR_H
#define USF_LOGIC_OP_EXECUTOR_H
#include "cpe/utils/stream.h"
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*action*/
logic_executor_t
logic_executor_action_create(logic_manage_t mgr, logic_executor_type_t type, cfg_t args);

/*composite*/
logic_executor_t
logic_executor_composite_create(logic_manage_t mgr, logic_executor_composite_type_t composite_type);

int logic_executor_composite_add(logic_executor_t composite, logic_executor_t member);

/*decorator*/
logic_executor_t
logic_executor_decorator_create(logic_manage_t mgr, logic_executor_decorator_type_t decorator_type, logic_executor_t inner);

/*common operations*/
const char * logic_executor_name(logic_executor_t executor);
void logic_executor_free(logic_executor_t executor);
void logic_executor_dump(logic_executor_t executor, write_stream_t stream, int level);

#ifdef __cplusplus
}
#endif

#endif

