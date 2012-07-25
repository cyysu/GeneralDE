#ifndef CPE_DR_METALIB_CMP_H
#define CPE_DR_METALIB_CMP_H
#include <stdio.h>
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int dr_macro_eq(LPDRMACRO l, LPDRMACRO r);
int dr_meta_eq(LPDRMETA l, LPDRMETA r);
int dr_entry_eq(LPDRMETAENTRY l, LPDRMETAENTRY r);
int dr_metalib_eq(LPDRMETALIB l, LPDRMETALIB r);
int dr_metalib_contains(LPDRMETALIB full, LPDRMETALIB part);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
