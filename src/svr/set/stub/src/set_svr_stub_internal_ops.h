#ifndef SVR_SET_STUB_INTERNAL_OPS_H
#define SVR_SET_STUB_INTERNAL_OPS_H
#include "set_svr_stub_internal_types.h"

/*svr oprations*/
set_svr_stub_t
set_svr_stub_create(
    gd_app_context_t app,
    const char * name, center_agent_t agent,
    center_agent_svr_type_t svr_type, uint16_t svr_id,
    mem_allocrator_t alloc, error_monitor_t em);
void set_svr_stub_free(set_svr_stub_t svr);

struct set_svr_stub_dispach_info *
set_svr_stub_find_dispatch_info_check_create(set_svr_stub_t svr, uint16_t svr_type);
struct set_svr_stub_dispach_info *
set_svr_stub_find_dispatch_info(set_svr_stub_t svr, uint16_t svr_type);

int set_svr_stub_outgoing_recv(dp_req_t req, void * ctx, error_monitor_t em);
ptr_int_t set_svr_stub_tick(void * ctx, ptr_int_t arg);

#endif
