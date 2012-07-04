#ifndef USF_BPG_REQ_MANAGE_H
#define USF_BPG_REQ_MANAGE_H
#include "bpg_req_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_req_manage_t
bpg_req_manage_create(
    logic_manage_t logic,
    const char * name,
    mem_allocrator_t alloc);

void bpg_req_manage_free(bpg_req_manage_t mgr);

bpg_req_manage_t
bpg_req_manage_find(gd_app_context_t app, cpe_hash_string_t name);

bpg_req_manage_t
bpg_req_manage_find_nc(gd_app_context_t app, const char * name);

logic_manage_t bpg_req_manage_logic(bpg_req_manage_t mgr);
gd_app_context_t bpg_req_manage_app(bpg_req_manage_t mgr);
const char * bpg_req_manage_name(bpg_req_manage_t mgr);
cpe_hash_string_t bpg_req_manage_name_hs(bpg_req_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif
