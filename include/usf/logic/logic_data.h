#ifndef USF_LOGIC_DATA_H
#define USF_LOGIC_DATA_H
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_data_t logic_data_find(logic_context_t context, const char * name);
logic_data_t logic_data_get_or_create(logic_context_t context, LPDRMETA meta, size_t capacity);
void logic_data_free(logic_data_t data);

LPDRMETA logic_data_meta(logic_data_t data);
const char * logic_data_name(logic_data_t data);
void * logic_data_data(logic_data_t data);
size_t logic_data_capacity(logic_data_t data);

int logic_data_try_read_int8(int8_t * result, logic_context_t context, const char * path, error_monitor_t em);
int logic_data_try_read_uint8(uint8_t * result, logic_context_t context, const char * path, error_monitor_t em);
int logic_data_try_read_int16(int16_t * result, logic_context_t context, const char * path, error_monitor_t em);
int logic_data_try_read_uint16(uint16_t * result, logic_context_t context, const char * path, error_monitor_t em);
int logic_data_try_read_int32(int32_t * result, logic_context_t context, const char * path, error_monitor_t em);
int logic_data_try_read_uint32(uint32_t * result, logic_context_t context, const char * path, error_monitor_t em);
int logic_data_try_read_int64(int64_t * result, logic_context_t context, const char * path, error_monitor_t em);
int logic_data_try_read_uint64(uint64_t * result, logic_context_t context, const char * path, error_monitor_t em);
int logic_data_try_read_float(float * result, logic_context_t context, const char * path, error_monitor_t em);
int logic_data_try_read_double(double * result, logic_context_t context, const char * path, error_monitor_t em);

int8_t logic_data_read_int8(logic_context_t context, const char * path);
uint8_t logic_data_read_uint8(logic_context_t context, const char * path);
int16_t logic_data_read_int16(logic_context_t context, const char * path);
uint16_t logic_data_read_uint16(logic_context_t context, const char * path);
int32_t logic_data_read_int32(logic_context_t context, const char * path);
uint32_t logic_data_read_uint32(logic_context_t context, const char * path);
int64_t logic_data_read_int64(logic_context_t context, const char * path);
uint64_t logic_data_read_uint64(logic_context_t context, const char * path);
float logic_data_read_float(logic_context_t context, const char * path);
double logic_data_read_double(logic_context_t context, const char * path);
const char * logic_data_read_string(logic_context_t context, const char * path);

int8_t logic_data_read_with_dft_int8(logic_context_t context, const char * path, int8_t dft);
uint8_t logic_data_read_with_dft_uint8(logic_context_t context, const char * path, uint8_t dft);
int16_t logic_data_read_with_dft_int16(logic_context_t context, const char * path, int16_t dft);
uint16_t logic_data_read_with_dft_uint16(logic_context_t context, const char * path, uint16_t dft);
int32_t logic_data_read_with_dft_int32(logic_context_t context, const char * path, int32_t dft);
uint32_t logic_data_read_with_dft_uint32(logic_context_t context, const char * path, uint32_t dft);
int64_t logic_data_read_with_dft_int64(logic_context_t context, const char * path, int64_t dft);
uint64_t logic_data_read_with_dft_uint64(logic_context_t context, const char * path, uint64_t dft);
float logic_data_read_with_dft_float(logic_context_t context, const char * path, float dft);
double logic_data_read_with_dft_double(logic_context_t context, const char * path, double dft);
const char * logic_data_read_with_dft_string(logic_context_t context, const char * path, const char * dft);

#ifdef __cplusplus
}
#endif

#endif

