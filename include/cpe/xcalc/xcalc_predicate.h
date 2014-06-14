#ifndef CPE_XCALC_PREDICATE_H
#define CPE_XCALC_PREDICATE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "xcalc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

xpredicate_t xpredicate_parse(mem_allocrator_t alloc, const char * exp, error_monitor_t em);
void xpredicate_free(mem_allocrator_t alloc, xpredicate_t pred);

int xpredicate_eval(xpredicate_t pred);

#ifdef __cplusplus
}
#endif

#endif

