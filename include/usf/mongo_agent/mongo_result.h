#ifndef USF_MONGO_RESULT_H
#define USF_MONGO_RESULT_H
#include "mongo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

mongo_result_t
mongo_result_get(mongo_agent_t agent, logic_require_t require);

void mongo_result_free(mongo_result_t result);

mongo_agent_t mongo_result_agent(mongo_result_t result);

LPDRMETA mongo_result_meta(mongo_result_t result);
void * mongo_result_data(mongo_result_t result);

uint32_t mongo_result_record_count(mongo_result_t result);
LPDRMETA mongo_result_record_capacity(mongo_result_t result);
void * mongo_result_record_data(mongo_result_t result, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif
