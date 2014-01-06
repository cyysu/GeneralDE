#ifndef CPE_MD5_H
#define CPE_MD5_H
#include "cpe/pal/pal_types.h"
#include "stream.h"
#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t digest[16];
} cpe_md5_value, * cpe_md5_value_t;

typedef struct cpe_md5_ctx {
    uint32_t i[2];
    uint32_t buf[4];
    uint8_t in[64];
    cpe_md5_value value;
} * cpe_md5_ctx_t;

void cpe_md5_ctx_init(cpe_md5_ctx_t ctx);
void cpe_md5_ctx_update(cpe_md5_ctx_t ctx, void const * inBuf, size_t inLen);
void cpe_md5_ctx_final(cpe_md5_ctx_t ctx);

void cpe_md5_print(write_stream_t s, cpe_md5_value_t value);
const char * cpe_md5_dump(cpe_md5_value_t value, mem_buffer_t buff);

#ifdef __cplusplus
}
#endif

#endif
