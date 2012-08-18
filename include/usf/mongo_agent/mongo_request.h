#ifndef USF_MONGO_REQUEST_H
#define USF_MONGO_REQUEST_H
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "mongo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern cpe_hash_string_t mongo_request_type_name;

mongo_request_t
mongo_request_create(mongo_agent_t agent, size_t capacity);

void mongo_request_free(mongo_request_t pkg);

mongo_agent_t mongo_request_agent(mongo_request_t pkg);

size_t mongo_request_set_meta(mongo_request_t pkg, LPDRMETA meta);
LPDRMETA mongo_request_meta(mongo_request_t pkg);
size_t mongo_request_data_size(mongo_request_t pkg);

dp_req_t mongo_request_to_dp_req(mongo_request_t pkg);
mongo_request_t mongo_request_from_dp_req(dp_req_t pkg);

uint32_t mongo_request_record_num(mongo_request_t pkg);
void * mongo_request_record_append(mongo_request_t pkg);

void mongo_request_init(mongo_request_t pkg);
void mongo_request_clear_data(mongo_request_t pkg);

/*pkg operations*/
uint32_t mongo_request_cmd(mongo_request_t pkg);
void mongo_request_set_cmd(mongo_request_t pkg, uint32_t cmd);

const char * mongo_request_dump(mongo_request_t req, mem_buffer_t buffer);
int mongo_request_build_from_cfg(mongo_request_t req, cfg_t cfg, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
