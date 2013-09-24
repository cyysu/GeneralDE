#ifndef CPE_ANDROID_FILE_H
#define CPE_ANDROID_FILE_H

#ifdef ANDROID

#include "cpe/pal/pal_types.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t android_file_load_to_buf(char * buf, size_t size, FILE * fp, error_monitor_t em);
ssize_t android_fild_load_to_buffer(mem_buffer_t buffer, FILE * fp, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif /*CPE_SUPPORT_ANDROID*/

#endif
