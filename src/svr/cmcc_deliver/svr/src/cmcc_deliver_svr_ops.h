#ifndef SVR_APPLE_IAP_SVR_OPS_H
#define SVR_APPLE_IAP_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "svr/center/agent/center_agent_types.h" 
#include "protocol/svr/cmcc_deliver/svr_cmcc_deliver_internal.h"
#include "cmcc_deliver_svr_types.h"

/*operations of cmcc_deliver_svr */
cmcc_deliver_svr_t
cmcc_deliver_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em);

void cmcc_deliver_svr_free(cmcc_deliver_svr_t svr);

cmcc_deliver_svr_t cmcc_deliver_svr_find(gd_app_context_t app, cpe_hash_string_t name);
cmcc_deliver_svr_t cmcc_deliver_svr_find_nc(gd_app_context_t app, const char * name);
const char * cmcc_deliver_svr_name(cmcc_deliver_svr_t svr);

int cmcc_deliver_svr_set_send_to(cmcc_deliver_svr_t svr, const char * send_to);
int cmcc_deliver_svr_set_request_recv_at(cmcc_deliver_svr_t svr, const char * name);

/*cmcc_deliver request ops*/
void cmcc_deliver_svr_op_deliver(cmcc_deliver_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

#endif
