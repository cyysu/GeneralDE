#ifndef USF_MONGO_REQUEST_H
#define USF_MONGO_REQUEST_H
#include "mongo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

mongo_request_t
mongo_request_get(
    mongo_agent_t agent,
    const char * table_name);

void mongo_request_free(mongo_request_t request);

mongo_agent_t mongo_request_agent(mongo_request_t request);
mongo_table_t mongo_request_table(mongo_request_t request);

int mongo_request_add_record(mongo_request_t request, LPDRMETA meta, const void * data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
