#ifndef SVR_SET_SVR_STUB_H
#define SVR_SET_SVR_STUB_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "set_svr_stub_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_svr_stub_t set_svr_stub_find_nc(gd_app_context_t app, const char * name);
set_svr_stub_t set_svr_stub_find(gd_app_context_t app, cpe_hash_string_t name);

gd_app_context_t set_svr_stub_app(set_svr_stub_t mgr);
const char * set_svr_stub_name(set_svr_stub_t mgr);
cpe_hash_string_t set_svr_stub_name_hs(set_svr_stub_t mgr);

cpe_hash_string_t set_svr_stub_request_dispatch_to(set_svr_stub_t svr);
int set_svr_stub_set_request_dispatch_to(set_svr_stub_t svr, const char * dispatch_to);

cpe_hash_string_t set_svr_stub_svr_notify_dispatch_to(set_svr_stub_t svr, uint16_t svr_type);
int set_svr_stub_set_svr_notify_dispatch_to(set_svr_stub_t svr, uint16_t svr_type, const char * dispatch_to);

cpe_hash_string_t set_svr_stub_response_dispatch_to(set_svr_stub_t svr);
int set_svr_stub_set_response_dispatch_to(set_svr_stub_t svr, const char * dispatch_to);

cpe_hash_string_t set_svr_stub_svr_response_dispatch_to(set_svr_stub_t svr, uint16_t svr_type);
int set_svr_stub_set_svr_response_dispatch_to(set_svr_stub_t svr, uint16_t svr_type, const char * dispatch_to);

int set_svr_stub_set_outgoing_recv_at(set_svr_stub_t svr, const char * outgoing_recv_at);

void set_svr_stub_set_recv_capacity(set_svr_stub_t svr, size_t capacity);

int set_svr_stub_start(set_svr_stub_t svr, uint16_t port);
void set_svr_stub_stop(set_svr_stub_t svr);

const char * set_svr_stub_name(set_svr_stub_t svr);

int set_svr_stub_send_pkg(set_svr_stub_t svr, dp_req_t body);

#ifdef __cplusplus
}
#endif

#endif
