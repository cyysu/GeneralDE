#ifndef CPE_ANDROID_ASSET_H
#define CPE_ANDROID_ASSET_H

#ifdef ANDROID

#include <jni.h>
#include "cpe/pal/pal_types.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

int android_asset_set_mgr(JNIEnv* env, jobject assetManager);

ssize_t android_asset_load_to_buf(char * buf, size_t size, const char * file_name, error_monitor_t em);
ssize_t android_asset_load_to_buffer(mem_buffer_t buffer, const char * file_name, error_monitor_t em);
int android_asset_exists(const char * file_name, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif

#endif
