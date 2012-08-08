#ifndef USF_BPG_ZIP_CHANEL_H
#define USF_BPG_ZIP_CHANEL_H
#include "cpe/cfg/cfg_types.h"
#include "bpg_zip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_zip_chanel_t
bpg_zip_chanel_create(
    gd_app_context_t app,
    const char * name,
    error_monitor_t em);

void bpg_zip_chanel_free(bpg_zip_chanel_t chanel);

bpg_zip_chanel_t
bpg_zip_chanel_find(gd_app_context_t app, cpe_hash_string_t name);
bpg_zip_chanel_t
bpg_zip_chanel_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t bpg_zip_chanel_app(bpg_zip_chanel_t chanel);
const char * bpg_zip_chanel_name(bpg_zip_chanel_t chanel);
cpe_hash_string_t bpg_zip_chanel_name_hs(bpg_zip_chanel_t chanel);

int bpg_zip_chanel_set_zip_send_to(bpg_zip_chanel_t chanel, cfg_t cfg);
int bpg_zip_chanel_set_zip_recv_at(bpg_zip_chanel_t chanel, const char * name);

int bpg_zip_chanel_set_unzip_send_to(bpg_zip_chanel_t chanel, cfg_t cfg);
int bpg_zip_chanel_set_unzip_recv_at(bpg_zip_chanel_t chanel, const char * name);

#ifdef __cplusplus
}
#endif

#endif

