#ifdef ANDROID
#include "cpe/pal/pal_signal.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/android/android_env.h"
#include "cpe/android/android_asset.h"
#include "gd/app/app_context.h"
#include "app_internal_types.h"

/*
#if __ANDROID_API__ > 8

int gd_app_cfg_reload(gd_app_context_t context) {
    /*
    if (android_asset_exists("etc.bc", context->m_em)) {
        struct mem_buffer buffer;
        ssize_t sz;

        mem_buffer_init(&buffer, NULL);

        sz = android_asset_load_to_buffer(&buffer, "etc.bc", context->m_em);
        if (sz < 0) {
            CPE_INFO(context->m_em, "load config from etc.bc fail!");
            mem_buffer_clear(&buffer);
            return -1;
        }

        if (cfg_read_bin(context->m_cfg, mem_buffer_make_continuous(&buffer, 0), sz, context->m_em) != 0) {
            CPE_INFO(context->m_em, "parse config from etc.bc fail!");
            mem_buffer_clear(&buffer);
            return -1;
        }

        if (context->m_debug) {
            CPE_INFO(context->m_em, "load config from etc.bc success!");
        }

        mem_buffer_clear(&buffer);
        return 0;
    }
    else * /
    if (android_asset_exists("etc.yml", context->m_em)) {
        struct mem_buffer buffer;
        struct read_stream_mem stream;
        ssize_t sz;

        mem_buffer_init(&buffer, NULL);

        sz = android_asset_load_to_buffer(&buffer, "etc.yml", context->m_em);
        if (sz < 0) {
            CPE_INFO(context->m_em, "load config from etc.yml fail!");
            mem_buffer_clear(&buffer);
            return -1;
        }

        read_stream_mem_init(&stream, mem_buffer_make_continuous(&buffer, 0), sz);

        if (cfg_read(context->m_cfg, (read_stream_t)&stream, cfg_merge_use_new, context->m_em) != 0) {
            CPE_INFO(context->m_em, "parse config from etc.yml fail!");
            mem_buffer_clear(&buffer);
            return -1;
        }

        if (context->m_debug) {
            CPE_INFO(context->m_em, "load config from etc.yml success!");
        }

        mem_buffer_clear(&buffer);
        return 0;
    }
    else {
        if (context->m_debug) {
            CPE_INFO(context->m_em, "load config from etc: dir not exist, skip!");
        }
        return -1;
    }
}
*/

//#else

int gd_app_cfg_reload(gd_app_context_t context) {
    int rv;
    const char * apk_name;
    cpe_unzip_context_t zip_context;
    cpe_unzip_dir_t zip_dir;
    cpe_unzip_file_t zip_file;

    apk_name = android_current_apk();
    if (strcmp(apk_name, "") == 0) {
        CPE_ERROR(context->m_em, "load config from assets/etc: apk config not exist!");
        return -1;
    }

    zip_context = cpe_unzip_context_create(apk_name, context->m_alloc, context->m_em);
    if (zip_context == NULL) {
        CPE_ERROR(context->m_em, "load config from %s:assets/etc: open apk fail!", apk_name);
        return -1;
    }
    //*
    if ((zip_file = cpe_unzip_file_find(zip_context, "assets/etc.bc", context->m_em))) {
      CPE_INFO(context->m_em, "etc.bc found");
        rv = cfg_read_zip_bin_file(
            context->m_cfg,
            zip_file,
            cfg_merge_use_new,
            context->m_em);

        if (rv == 0){
            if (context->m_debug) {
                CPE_INFO(context->m_em, "load config from %s:assets/etc.bc success!", apk_name);
            }

            if ((zip_file = cpe_unzip_file_find(zip_context, "assets/etc.android.bc", context->m_em))) {
                CPE_INFO(context->m_em, "etc.android.bc found");
                rv = cfg_read_zip_bin_file(
                    context->m_cfg,
                    zip_file,
                    cfg_merge_use_new,
                    context->m_em);
                if (rv == 0){
                    if (context->m_debug) {
                        CPE_INFO(context->m_em, "load config from %s:assets/etc.android.bc success!", apk_name);
                    }
                }else{
                    CPE_INFO(context->m_em, "etc.android.bc load failed");
                }
            }
        }else{
            CPE_INFO(context->m_em, "etc.bc load failed");
        }
    }
    else //*/
    if ((zip_file = cpe_unzip_file_find(zip_context, "assets/etc.yml", context->m_em))) {
        rv = cfg_read_zip_file(
            context->m_cfg,
            zip_file,
            cfg_merge_use_new,
            context->m_em);

        if (rv == 0) {
            if (context->m_debug) {
                CPE_INFO(context->m_em, "load config from %s:assets/etc.yml success!", apk_name);
            }
            if ((zip_file = cpe_unzip_file_find(zip_context, "assets/etc.android.yml", context->m_em))) {
                rv = cfg_read_zip_file(
                    context->m_cfg,
                    zip_file,
                    cfg_merge_use_new,
                    context->m_em);
                if (rv == 0) {
                    if (context->m_debug) {
                        CPE_INFO(context->m_em, "load config from %s:assets/etc.android.yml success!", apk_name);
                    }
                }else{

                    CPE_INFO(context->m_em, "etc.android.yml load failed");
                }
            }
        }else{
            CPE_INFO(context->m_em, "etc.yml load failed");
        }
    }
    else if ((zip_dir = cpe_unzip_dir_find(zip_context, "assets/etc", context->m_em))) {
        rv = cfg_read_zip_dir(
            context->m_cfg,
            zip_dir,
            cfg_merge_use_new,
            context->m_em,
            context->m_alloc);

        if (rv == 0) {
            if (context->m_debug) {
                CPE_INFO(context->m_em, "load config from %s:assets/etc success!", apk_name);
            }
            if ((zip_dir = cpe_unzip_dir_find(zip_context, "assets/etc.android", context->m_em))) {
                rv = cfg_read_zip_dir(
                    context->m_cfg,
                    zip_dir,
                    cfg_merge_use_new,
                    context->m_em,
                    context->m_alloc);
                if (rv == 0) {
                    if (context->m_debug) {
                        CPE_INFO(context->m_em, "load config from %s:assets/etc.android success!", apk_name);
                    }
                }else{
                    CPE_INFO(context->m_em, "etc.android load failed");
                }
            }
        }else{
            CPE_INFO(context->m_em, "etc load failed");
        }
    }
    else {
      //if (context->m_debug) {
            CPE_INFO(context->m_em, "load config from %s:assets/etc: dir not exist, skip!", apk_name);
	    // }
        rv = 0;
    }

    cpe_unzip_context_free(zip_context);

    return rv;
}

//#endif /* __ANDROID_API__ */

#endif /*ANDROID*/

