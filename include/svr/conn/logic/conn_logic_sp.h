#ifndef SVR_CONN_LOGIC_SP_H
#define SVR_CONN_LOGIC_SP_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "conn_logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

conn_logic_sp_t conn_logic_sp_create(
    gd_app_context_t app,
    const char * name, 
    logic_manage_t mgr,
    mem_allocrator_t alloc, error_monitor_t em);

void conn_logic_sp_free(conn_logic_sp_t mgr);

conn_logic_sp_t conn_logic_sp_find(gd_app_context_t app, cpe_hash_string_t name);
conn_logic_sp_t conn_logic_sp_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t conn_logic_sp_app(conn_logic_sp_t mgr);
const char * conn_logic_sp_name(conn_logic_sp_t mgr);
cpe_hash_string_t conn_logic_sp_name_hs(conn_logic_sp_t mgr);

int conn_logic_sp_set_outgoing_dispatch_to(conn_logic_sp_t sp, const char * outgoing_dispatch_to);
cpe_hash_string_t conn_logic_sp_outgoing_dispatch_to(conn_logic_sp_t sp);
int conn_logic_sp_set_incoming_recv_at(conn_logic_sp_t sp, const char * incoming_recv_at);

int conn_logic_sp_send_request(
    conn_logic_sp_t sp,
    LPDRMETA meta, void const * data, size_t data_size,
    logic_require_t require);

dp_req_t conn_logic_sp_outgoing_buf(conn_logic_sp_t sp, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
