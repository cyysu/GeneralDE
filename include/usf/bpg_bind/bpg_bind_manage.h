#ifndef USF_BPG_BIND_MANAGE_H
#define USF_BPG_BIND_MANAGE_H
#include "cpe/cfg/cfg_types.h"
#include "bpg_bind_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_bind_manage_t
bpg_bind_manage_create(
    gd_app_context_t app,
    mem_allocrator_t alloc,
    const char * name,
    bpg_rsp_manage_t rsp_mgr,
    error_monitor_t em);

void bpg_bind_manage_free(bpg_bind_manage_t mgr);

bpg_bind_manage_t
bpg_bind_manage_find(gd_app_context_t app, cpe_hash_string_t name);
bpg_bind_manage_t
bpg_bind_manage_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t bpg_bind_manage_app(bpg_bind_manage_t mgr);
const char * bpg_bind_manage_name(bpg_bind_manage_t mgr);
cpe_hash_string_t bpg_bind_manage_name_hs(bpg_bind_manage_t mgr);

int bpg_bind_manage_set_recv_at(bpg_bind_manage_t mgr, const char * name);
bpg_rsp_manage_t bpg_bind_manage_rsp_manage(bpg_bind_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif

