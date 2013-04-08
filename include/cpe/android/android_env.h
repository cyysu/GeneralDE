#ifndef CPE_ANDROID_ENV_H
#define CPE_ANDROID_ENV_H

#ifdef ANDROID

#include "cpe/utils/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/*log operations*/
void android_cpe_error_log_to_log(struct error_info * info, void * context, const char * fmt, va_list args);

void android_set_current_apk(const char * full_apk_name);
const char * android_current_apk(void);

#ifdef __cplusplus
}
#endif

#endif /*CPE_SUPPORT_ANDROID*/

#endif
