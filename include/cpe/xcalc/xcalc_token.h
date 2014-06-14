#ifndef CPE_XCALC_TOKEN_H
#define CPE_XCALC_TOKEN_H
#include "cpe/utils/stream.h"
#include "xcalc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xtoken_data {
    union {
        int64_t _int;
        double _double;
    } num;
    struct {
        char * _string;
        char * _end;
    } str;
};

struct xtoken {
    uint32_t m_type;
    struct xtoken_data m_data;
    xtoken_t m_sub;
};

int32_t xtoken_to_int32(xtoken_t token);
int64_t xtoken_to_int64(xtoken_t token);

void xtoken_dump(write_stream_t s, xtoken_t token);

#ifdef __cplusplus
}
#endif

#endif
