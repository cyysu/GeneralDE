#ifndef SVR_SET_CHANEL_SHM_H
#define SVR_SET_CHANEL_SHM_H
#include "cpe/utils/buffer.h"
#include "set_share_types.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * set_repository_root(gd_app_context_t app, char * buf, size_t buf_capacity, error_monitor_t em);
set_chanel_t set_repository_chanel_open(gd_app_context_t app, const char * svr_type_name, uint16_t svr_id, error_monitor_t em);
set_chanel_t set_repository_chanel_attach(gd_app_context_t app, const char * svr_type_name, uint16_t svr_id, error_monitor_t em);

int set_repository_chanel_destory(gd_app_context_t app, const char * svr_type_name, uint16_t svr_id);
void set_repository_chanel_detach(set_chanel_t chanel, error_monitor_t em);

int set_repository_search(
    gd_app_context_t app,
    void (*on_find_svr)(void * ctx, const char * svr_type, uint16_t svr_id), void * ctx,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
