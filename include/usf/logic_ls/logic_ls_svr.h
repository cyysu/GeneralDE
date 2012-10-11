#ifndef USF_LOGIC_LS_SVR_H
#define USF_LOGIC_LS_SVR_H
#include "logic_ls_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_local_svr_t
logic_local_svr_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void logic_local_svr_free(logic_local_svr_t ls);

logic_local_svr_t
logic_local_svr_find(gd_app_context_t app, cpe_hash_string_t name);

logic_local_svr_t
logic_local_svr_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t logic_local_svr_app(logic_local_svr_t mgr);
error_monitor_t logic_local_svr_em(logic_local_svr_t mgr);

const char * logic_local_svr_name(logic_local_svr_t mgr);
cpe_hash_string_t logic_local_svr_name_hs(logic_local_svr_t mgr);

logic_context_t
logic_local_svr_create_context(
    logic_local_svr_t mgr,
    const char * rsp_name);

int logic_local_svr_execute(
    logic_local_svr_t mgr,
    logic_context_t context,
    logic_require_t require);

#ifdef __cplusplus
}
#endif

#endif
