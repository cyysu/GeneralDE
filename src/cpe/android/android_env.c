#ifdef ANDROID
#include <android/log.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/android/android_env.h"

void android_cpe_error_log_to_log(struct error_info * info, void * context, const char * fmt, va_list args) {
    char buf[1024];
    int buf_pos = 0;
    int prio = ANDROID_LOG_DEBUG;

    switch(info->m_level) {
    case CPE_EL_INFO:
        prio = ANDROID_LOG_INFO;
        break;
    case CPE_EL_WARNING:
        prio = ANDROID_LOG_WARN;
        break;
    case CPE_EL_ERROR:
        prio = ANDROID_LOG_ERROR;
        break;
    }

    if (info->m_file) {
        buf_pos += snprintf(buf + buf_pos, sizeof(buf) - buf_pos, "%s:%d: ", info->m_file, info->m_line > 0 ? info->m_line : 0);
    }
    else if (info->m_line >= 0) {
        buf_pos += snprintf(buf + buf_pos, sizeof(buf) - buf_pos, "%d: ", info->m_line);
    }

    buf_pos += vsnprintf(buf + buf_pos, sizeof(buf) - buf_pos, fmt, args);
    buf_pos += snprintf(buf + buf_pos, sizeof(buf) - buf_pos, "\n");

	__android_log_write(prio, "cpe", buf);
}

char g_android_apk[256] = { 0 };

void android_set_current_apk(const char * full_apk_name) {
    strncpy(g_android_apk, full_apk_name, sizeof(g_android_apk));
}

const char * android_current_apk(void) {
    return g_android_apk;
}

#endif
