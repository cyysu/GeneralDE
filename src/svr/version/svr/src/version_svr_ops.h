#ifndef SVR_DBLOG_SVR_OPS_H
#define SVR_DBLOG_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "version_svr_types.h"
#include "svr/center/agent/center_agent_types.h" 

/*operations of version_svr */
version_svr_t
version_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em);

void version_svr_free(version_svr_t svr);

version_svr_t version_svr_find(gd_app_context_t app, cpe_hash_string_t name);
version_svr_t version_svr_find_nc(gd_app_context_t app, const char * name);
const char * version_svr_name(version_svr_t svr);

int version_svr_set_send_to(version_svr_t svr, const char * send_to);
int version_svr_set_recv_at(version_svr_t svr, const char * name);
int version_svr_rsp(dp_req_t req, void * ctx, error_monitor_t em);

int version_svr_parse_version_cmp(SVR_VERSION_VERSION const * l, SVR_VERSION_VERSION const * r);
int version_svr_parse_version(SVR_VERSION_VERSION * version, const char * str);

void version_svr_send_error_response(version_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body, int err);

/*protocol process ops*/
typedef void (*version_svr_op_t)(version_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);
void version_svr_op_query_update(version_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);

#endif
