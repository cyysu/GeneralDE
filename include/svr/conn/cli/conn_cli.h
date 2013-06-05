#ifndef SVR_CONN_CLI_H
#define SVR_CONN_CLI_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "conn_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

conn_cli_t conn_cli_create(
    gd_app_context_t app,
    const char * name, 
    mem_allocrator_t alloc, error_monitor_t em);

void conn_cli_free(conn_cli_t mgr);

conn_cli_t conn_cli_find(gd_app_context_t app, cpe_hash_string_t name);
conn_cli_t conn_cli_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t conn_cli_app(conn_cli_t mgr);
const char * conn_cli_name(conn_cli_t mgr);
cpe_hash_string_t conn_cli_name_hs(conn_cli_t mgr);

int conn_cli_set_svr(conn_cli_t cli, const char * ip, uint16_t port);
const char * conn_cli_svr_ip(conn_cli_t cli);
uint16_t conn_cli_svr_port(conn_cli_t cli);

conn_cli_state_t conn_cli_state(conn_cli_t cli);
void conn_cli_enable(conn_cli_t cli);
void conn_cli_disable(conn_cli_t cli);

int conn_cli_send(
    conn_cli_t cli,
    uint16_t to_svr, uint32_t sn,
    LPDRMETA meta, void const * data, uint16_t data_len);

#ifdef __cplusplus
}
#endif

#endif
