#ifndef CPE_XCALC_COMPUTER_H
#define CPE_XCALC_SCANE_H
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "xcalc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

xcomputer_t xcomputer_create(mem_allocrator_t alloc, const char * str);
void xcomputer_free(xcomputer_t computer);

#ifdef __cplusplus
}
#endif

#endif
