#ifndef USF_MONGO_REQUEST_H
#define USF_MONGO_REQUEST_H
#include "bson.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
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

dp_req_t mongo_request_to_dp_req(mongo_request_t pkg);
mongo_request_t mongo_request_from_dp_req(dp_req_t pkg);

void mongo_request_init(mongo_request_t pkg);
size_t mongo_request_size(mongo_request_t pkg);
void * mongo_request_data(mongo_request_t pkg);
int mongo_request_set_size(mongo_request_t pkg, size_t size);
size_t mongo_request_capacity(mongo_request_t pkg);

/*pkg operations*/
uint32_t mongo_request_op(mongo_request_t pkg);
void mongo_request_set_op(mongo_request_t pkg, uint32_t cmd);

const char * mongo_request_dump(mongo_request_t req, mem_buffer_t buffer);
int mongo_request_build_from_cfg(mongo_request_t req, cfg_t cfg, error_monitor_t em);

int mongo_request_append_int32(mongo_request_t pkg, const char *name, const int32_t i);
int mongo_request_append_int64(mongo_request_t pkg, const char *name, const int64_t i);
int mongo_request_append_double(mongo_request_t pkg, const char *name, const double d);
int mongo_request_append_string(mongo_request_t pkg, const char *name, const char *str);
int mongo_request_append_string_n(mongo_request_t pkg, const char *name, const char *str, int len);
int mongo_request_append_symbol(mongo_request_t pkg, const char *name, const char *str);
int mongo_request_append_symbol_n(mongo_request_t pkg, const char *name, const char *str, int len);
int mongo_request_append_code(mongo_request_t pkg, const char *name, const char *str);
int mongo_request_append_code_n(mongo_request_t pkg, const char *name, const char *str, int len);
/* int mongo_request_append_code_w_scope(mongo_request_t pkg, const char *name, const char *code, const mongo_request *scope); */
/* int mongo_request_append_code_w_scope_n(mongo_request_t pkg, const char *name, const char *code, int size, const mongo_request *scope); */
int mongo_request_append_binary(mongo_request_t pkg, const char *name, char type, const char *str, int len);
int mongo_request_append_bool(mongo_request_t pkg, const char *name, const int v);
int mongo_request_append_null(mongo_request_t pkg, const char *name);
int mongo_request_append_undefined(mongo_request_t pkg, const char *name);
int mongo_request_append_regex(mongo_request_t pkg, const char *name, const char *pattern, const char *opts);
int mongo_request_append_timestamp(mongo_request_t pkg, const char *name, int time, int increment);
int mongo_request_append_date(mongo_request_t pkg, const char *name, int64_t millis);
int mongo_request_append_time_t(mongo_request_t pkg, const char *name, time_t secs);
int mongo_request_append_start_object(mongo_request_t pkg, const char *name);
int mongo_request_append_start_array(mongo_request_t pkg, const char *name);
int mongo_request_append_finish_object(mongo_request_t pkg);
int mongo_request_append_finish_array(mongo_request_t pkg);

#ifdef __cplusplus
}
#endif

#endif
