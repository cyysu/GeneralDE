#ifndef TSF4WG_TCAPLUS_INTERNAL_OPS_H
#define TSF4WG_TCAPLUS_INTERNAL_OPS_H
#include "mongo_internal_types.h"

/*agent operations*/
ptr_int_t mongo_agent_tick(void * ctx, ptr_int_t max_process_count);
int mongo_agent_save_require_id(mongo_agent_t agent, logic_require_id_t id);
int mongo_agent_remove_require_id(mongo_agent_t agent, logic_require_id_t id);
void mongo_agent_notify_all_require_disconnect(mongo_agent_t agent);

/*request operations*/
uint32_t mongo_request_hash(const struct mongo_request * context);
int mongo_request_cmp(const struct mongo_request * l, const struct mongo_request * r);
void mongo_request_free_all(mongo_agent_t agent);
void * mongo_request_data_buf(mongo_request_t request);
size_t mongo_request_data_capacity(mongo_request_t request);

/*resuilt operations*/
uint32_t mongo_result_hash(const struct mongo_result * context);
int mongo_result_cmp(const struct mongo_result * l, const struct mongo_result * r);
void mongo_result_free_all(mongo_agent_t agent);


#endif
