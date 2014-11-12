#ifndef CPE_PAL_STRINGS_H
#define CPE_PAL_STRINGS_H

#if defined _WIN32
#include <string.h>
# define bzero(p,l) memset((p),0,(l))
#else
#include <strings.h>
#endif

#endif
