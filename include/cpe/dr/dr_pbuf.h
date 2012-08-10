#ifndef CPE_DR_PBUF_H
#define CPE_DR_PBUF_H
#include "cpe/utils/stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int dr_pbuf_read(
    void * result,
    size_t capacity,
    const void * input,
    size_t input_capacity,
    LPDRMETA meta,
    error_monitor_t em);

int dr_pbuf_read_to_buffer(
    struct mem_buffer * result, 
    const void * input,
    size_t input_capacity,
    LPDRMETA meta,
    error_monitor_t em);

int dr_pbuf_write(
    void * output,
    size_t output_capacity,
    const void * input,
    size_t input_capacity,
    LPDRMETA meta,
    error_monitor_t em);

int cpe_dr_pbuf_encode32(uint32_t number, uint8_t buffer[10]);
int cpe_dr_pbuf_encode64(uint64_t number, uint8_t buffer[10]);
int cpe_dr_pbuf_zigzag32(int32_t number, uint8_t buffer[10]);
int cpe_dr_pbuf_zigzag64(int64_t number, uint8_t buffer[10]);

int cpe_dr_pbuf_decode_uint32(uint8_t buffer[10], uint32_t * number);
int cpe_dr_pbuf_decode_uint64(uint8_t buffer[10], uint64_t * number);
int cpe_dr_pbuf_decode_int32(uint8_t buffer[10], int32_t * number);
int cpe_dr_pbuf_deocde_int64(uint8_t buffer[10], int64_t * number);

#ifdef __cplusplus
}
#endif

#endif
