#ifndef USF_MONGO_AGENT_INTERNAL_OPS_H
#define USF_MONGO_AGENT_INTERNAL_OPS_H
#include "mongo_agent_internal_types.h"

/*agent operations*/
int mongo_agent_save_require_id(mongo_agent_t agent, logic_require_id_t id);
int mongo_agent_remove_require_id(mongo_agent_t agent, logic_require_id_t id);
void mongo_agent_notify_all_require_disconnect(mongo_agent_t agent);

#endif
