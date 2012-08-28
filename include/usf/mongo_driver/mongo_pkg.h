#ifndef USF_MONGO_DRIVER_REQUEST_H
#define USF_MONGO_DRIVER_REQUEST_H
#include "bson.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_types.h"
#include "mongo_driver_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern cpe_hash_string_t mongo_pkg_type_name;

mongo_pkg_t
mongo_pkg_create(mongo_driver_t agent, size_t capacity);

void mongo_pkg_free(mongo_pkg_t pkg);

mongo_driver_t mongo_pkg_driver(mongo_pkg_t pkg);

dp_req_t mongo_pkg_to_dp_req(mongo_pkg_t pkg);
mongo_pkg_t mongo_pkg_from_dp_req(dp_req_t pkg);

void mongo_pkg_init(mongo_pkg_t pkg);
size_t mongo_pkg_size(mongo_pkg_t pkg);
void * mongo_pkg_data(mongo_pkg_t pkg);
int mongo_pkg_set_size(mongo_pkg_t pkg, size_t size);
size_t mongo_pkg_capacity(mongo_pkg_t pkg);

/*pkg operations*/
uint32_t mongo_pkg_op(mongo_pkg_t pkg);
void mongo_pkg_set_op(mongo_pkg_t pkg, uint32_t cmd);

uint32_t mongo_pkg_id(mongo_pkg_t pkg);
void mongo_pkg_set_id(mongo_pkg_t pkg, uint32_t id);

const char *mongo_pkg_db(mongo_pkg_t pkg);
void mongo_pkg_set_db(mongo_pkg_t pkg, const char * db);

const char * mongo_pkg_dump(mongo_pkg_t req, mem_buffer_t buffer);
int mongo_pkg_build_from_cfg(mongo_pkg_t req, cfg_t cfg, error_monitor_t em);

int mongo_pkg_append_int32(mongo_pkg_t pkg, const char *name, const int32_t i);
int mongo_pkg_append_int64(mongo_pkg_t pkg, const char *name, const int64_t i);
int mongo_pkg_append_double(mongo_pkg_t pkg, const char *name, const double d);
int mongo_pkg_append_string(mongo_pkg_t pkg, const char *name, const char *str);
int mongo_pkg_append_string_n(mongo_pkg_t pkg, const char *name, const char *str, int len);
int mongo_pkg_append_symbol(mongo_pkg_t pkg, const char *name, const char *str);
int mongo_pkg_append_symbol_n(mongo_pkg_t pkg, const char *name, const char *str, int len);
int mongo_pkg_append_code(mongo_pkg_t pkg, const char *name, const char *str);
int mongo_pkg_append_code_n(mongo_pkg_t pkg, const char *name, const char *str, int len);
/* int mongo_pkg_append_code_w_scope(mongo_pkg_t pkg, const char *name, const char *code, const mongo_pkg *scope); */
/* int mongo_pkg_append_code_w_scope_n(mongo_pkg_t pkg, const char *name, const char *code, int size, const mongo_pkg *scope); */
int mongo_pkg_append_binary(mongo_pkg_t pkg, const char *name, char type, const char *str, int len);
int mongo_pkg_append_bool(mongo_pkg_t pkg, const char *name, const int v);
int mongo_pkg_append_null(mongo_pkg_t pkg, const char *name);
int mongo_pkg_append_undefined(mongo_pkg_t pkg, const char *name);
int mongo_pkg_append_regex(mongo_pkg_t pkg, const char *name, const char *pattern, const char *opts);
int mongo_pkg_append_timestamp(mongo_pkg_t pkg, const char *name, int time, int increment);
int mongo_pkg_append_date(mongo_pkg_t pkg, const char *name, int64_t millis);
int mongo_pkg_append_time_t(mongo_pkg_t pkg, const char *name, time_t secs);
int mongo_pkg_append_start_object(mongo_pkg_t pkg, const char *name);
int mongo_pkg_append_start_array(mongo_pkg_t pkg, const char *name);
int mongo_pkg_append_finish_object(mongo_pkg_t pkg);
int mongo_pkg_append_finish_array(mongo_pkg_t pkg);

#ifdef __cplusplus
}
#endif

#endif
