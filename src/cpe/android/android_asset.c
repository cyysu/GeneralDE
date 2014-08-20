#ifdef ANDROID

#include <stdlib.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "cpe/android/android_asset.h"

static AAssetManager * g_asset_mgr = NULL;

int android_asset_set_mgr(JNIEnv* env, jobject assetManager) {
    g_asset_mgr = AAssetManager_fromJava(env, assetManager);  

    if(g_asset_mgr == NULL) return -1;

    return 0;
}

ssize_t android_asset_load_to_buf(char * buf, size_t size, const char * file_name, error_monitor_t em) {
    AAsset* asset;
    off_t buff_size;
    off_t read_size;

    if(g_asset_mgr == NULL) {  
        CPE_ERROR(em, "AAssetManager not init!");
        return -1; 
    }
   
    /*获取文件名并打开*/  
    asset = AAssetManager_open(g_asset_mgr, file_name, AASSET_MODE_UNKNOWN);
    if(asset == NULL) {
        CPE_ERROR(em, "AAssetManager_open %s fail!", file_name);
        return -1;
    }

    buff_size = AAsset_getLength(asset);
    if (buff_size >= size) {
        CPE_ERROR(em, "AAsset %s size %d overflow, buf size is %d!", file_name, (int)buff_size, (int)size);
        AAsset_close(asset);
        return -1;
    }

    read_size = AAsset_read(asset, buf, buff_size);  
    if (read_size < buff_size) {
        CPE_ERROR(em, "AAsset %s read return not enouth, return size %d, require size %d!", file_name, (int)read_size, (int)buff_size);
        AAsset_close(asset);
        return -1;
    }

    AAsset_close(asset);
    return read_size;
}

ssize_t android_asset_load_to_buffer(mem_buffer_t buffer, const char * file_name, error_monitor_t em) {
    AAsset* asset;
    off_t buff_size;
    off_t read_size;
    void * buf;

    if(g_asset_mgr == NULL) {  
        CPE_ERROR(em, "AAssetManager not init!");
        return -1; 
    }
   
    /*获取文件名并打开*/  
    asset = AAssetManager_open(g_asset_mgr, file_name, AASSET_MODE_UNKNOWN);
    if(asset == NULL) {
        CPE_ERROR(em, "AAssetManager_open %s fail!", file_name);
        return -1;
    }

    buff_size = AAsset_getLength(asset);

    buf = mem_buffer_make_continuous(buffer, buff_size);
    if (buf == NULL) {
        CPE_ERROR(em, "AAsset %s alloc buff %d fail!", file_name, (int)buff_size);
        AAsset_close(asset);
        return -1;
    }

    read_size = AAsset_read(asset, buf, buff_size);  
    if (read_size < buff_size) {
        CPE_ERROR(em, "AAsset %s read return not enouth, return size %d, require size %d!", file_name, (int)read_size, (int)buff_size);
        AAsset_close(asset);
        return -1;
    }

    AAsset_close(asset);
    return read_size;
}

int android_asset_exists(const char * file_name, error_monitor_t em) {
    AAsset* asset;

    if(g_asset_mgr == NULL) {  
        CPE_ERROR(em, "AAssetManager not init!");
        return -1; 
    }
   
    /*获取文件名并打开*/  
    asset = AAssetManager_open(g_asset_mgr, file_name, AASSET_MODE_UNKNOWN);
    if(asset == NULL) {
        return 0;
    }

    AAsset_close(asset);
    return 1;
}

#endif
