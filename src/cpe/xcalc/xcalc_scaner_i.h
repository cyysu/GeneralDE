#ifndef CPE_XCALC_SCANER_I_H
#define CPE_XCALC_SCANER_I_H
#include "cpe/xcalc/xcalc_scaner.h"
#include "xcalc_token_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xscaner {
    mem_allocrator_t m_alloc;
    char * m_buf;
    char * m_cur_pos;
};

#ifdef __cplusplus
}
#endif

#endif
