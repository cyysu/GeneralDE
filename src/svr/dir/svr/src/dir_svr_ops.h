#ifndef SVR_DBLOG_SVR_OPS_H
#define SVR_DBLOG_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "dir_svr_types.h"
#include "svr/center/agent/center_agent_types.h" 

/*operations of dir_svr */
dir_svr_t
dir_svr_create(
    gd_app_context_t app,
    const char * name,
    uint16_t game_id,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em);

void dir_svr_free(dir_svr_t svr);

dir_svr_t dir_svr_find(gd_app_context_t app, cpe_hash_string_t name);
dir_svr_t dir_svr_find_nc(gd_app_context_t app, const char * name);
const char * dir_svr_name(dir_svr_t svr);

int dir_svr_set_send_to(dir_svr_t svr, const char * send_to);
int dir_svr_set_recv_at(dir_svr_t svr, const char * name);
int dir_svr_rsp(dp_req_t req, void * ctx, error_monitor_t em);

void dir_svr_send_error_response(dir_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body, int err);

/*region ops*/
dir_svr_region_t dir_svr_region_create(dir_svr_t svr, uint16_t region_id, const char * region_name, uint8_t region_state);
void dir_svr_region_free(dir_svr_region_t region);

/*server ops*/
dir_svr_server_t dir_svr_server_create(dir_svr_region_t region, const char * ip, uint16_t port);
void dir_svr_server_free(dir_svr_server_t server);

/*protocol process ops*/
typedef void (*dir_svr_op_t)(dir_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);
void dir_svr_op_query_regions(dir_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);
void dir_svr_op_query_servers(dir_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);

#endif
