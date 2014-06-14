#ifndef CPE_XCALC_SCANE_H
#define CPE_XCALC_SCANE_H
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "xcalc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

xscaner_t xscaner_create(mem_allocrator_t alloc, const char * str);
void xscaner_free(xscaner_t scaner);

int xscaner_get_token(xscaner_t scaner, xtoken_t token, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
