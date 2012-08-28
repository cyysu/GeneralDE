#ifndef USF_MONGO_AGENT_INTERNAL_OPS_H
#define USF_MONGO_AGENT_INTERNAL_OPS_H
#include "mongo_agent_internal_types.h"

int mongo_agent_recv(dp_req_t req, void * ctx, error_monitor_t em);

#endif
