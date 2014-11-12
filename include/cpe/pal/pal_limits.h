#ifndef CPE_PAL_LIMITS_H
#define CPE_PAL_LIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 /*windows*/

#ifndef UINT16_MAX
    #define UINT16_MAX  0xFFFFu
#endif

#ifndef UINT32_MAX
    #define UINT32_MAX  0xFFFFFFFFUL
#endif

#else /*ux*/

#include <limits.h>

#endif

#ifdef __cplusplus
}
#endif

#endif

