#ifndef CPE_PAL_FCNTL_H
#define CPE_PAL_FCNTL_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined _MSC_VER /*windows*/
#include <windows.h>
#include <fcntl.h> 
#include <errno.h>

#else
/*posix */

#include <fcntl.h> 
#include <errno.h>

#define O_BINARY 0

#endif

#ifdef __cplusplus
}
#endif

#endif

