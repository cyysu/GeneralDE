#ifndef TSF4WG_TCAPLUS_INTERNAL_OPS_H
#define TSF4WG_TCAPLUS_INTERNAL_OPS_H
#include "mongo_internal_types.h"

/*agent operations*/
ptr_int_t mongo_agent_tick(void * ctx, ptr_int_t max_process_count);
int mongo_agent_save_require_id(mongo_agent_t agent, logic_require_id_t id);
int mongo_agent_remove_require_id(mongo_agent_t agent, logic_require_id_t id);
void mongo_agent_notify_all_require_disconnect(mongo_agent_t agent);

#endif
