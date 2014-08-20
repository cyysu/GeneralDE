#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_data_value.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"

int8_t dr_value_read_with_dft_int8(dr_value_t value, int8_t dft) {
    int8_t r;
    return dr_ctype_try_read_int8(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

uint8_t dr_value_read_with_dft_uint8(dr_value_t value, uint8_t dft) {
    uint8_t r;
    return dr_ctype_try_read_uint8(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

int16_t dr_value_read_with_dft_int16(dr_value_t value, int16_t dft) {
    int16_t r;
    return dr_ctype_try_read_int16(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

uint16_t dr_value_read_with_dft_uint16(dr_value_t value, uint16_t dft) {
    uint16_t r;
    return dr_ctype_try_read_uint16(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

int32_t dr_value_read_with_dft_int32(dr_value_t value, int32_t dft) {
    int32_t r;
    return dr_ctype_try_read_int32(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

uint32_t dr_value_read_with_dft_uint32(dr_value_t value, uint32_t dft) {
    uint32_t r;
    return dr_ctype_try_read_uint32(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

int64_t dr_value_read_with_dft_int64(dr_value_t value, int64_t dft) {
    int64_t r;
    return dr_ctype_try_read_int64(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

uint64_t dr_value_read_with_dft_uint64(dr_value_t value, uint64_t dft) {
    uint64_t r;
    return dr_ctype_try_read_uint64(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

float dr_value_read_with_dft_float(dr_value_t value, float dft) {
    float r;
    return dr_ctype_try_read_float(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

double dr_value_read_with_dft_double(dr_value_t value, double dft) {
    double r;
    return dr_ctype_try_read_double(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

const char * dr_value_read_with_dft_string(dr_value_t value, const char * dft) {
    return (value->m_type == CPE_DR_TYPE_STRING) ? value->m_data : dft;
}

int dr_value_try_read_int8(int8_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_int8(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_uint8(uint8_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_uint8(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_int16(int16_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_int16(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_uint16(uint16_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_uint16(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_int32(int32_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_int32(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_uint32(uint32_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_uint32(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_int64(int64_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_int64(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_uint64(uint64_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_uint64(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_float(float * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_float(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_double(double * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_double(result, value->m_data, value->m_type, em);
}

const char * dr_value_try_read_string(dr_value_t value, error_monitor_t em) {
    return (value->m_type == CPE_DR_TYPE_STRING) ? value->m_data : NULL;
}

