#ifndef SVR_CONN_CLI_PKG_H
#define SVR_CONN_CLI_PKG_H
#include "cpe/utils/buffer.h"
#include "conn_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * req_type_conn_cli_pkg;

conn_cli_pkg_t conn_cli_pkg_create(conn_cli_t cli);
void conn_cli_pkg_free(conn_cli_pkg_t pkg);
conn_cli_t conn_cli_pkg_cli(conn_cli_pkg_t pkg);

void conn_cli_pkg_init(conn_cli_pkg_t pkg);

uint16_t conn_cli_pkg_svr_type(conn_cli_pkg_t pkg);
void conn_cli_pkg_set_svr_type(conn_cli_pkg_t pkg, uint16_t svr_type);

uint8_t conn_cli_pkg_result(conn_cli_pkg_t pkg);
void conn_cli_pkg_set_result(conn_cli_pkg_t pkg, uint8_t result);

uint8_t conn_cli_pkg_flags(conn_cli_pkg_t pkg);
void conn_cli_pkg_set_flags(conn_cli_pkg_t pkg, uint8_t flags);

uint32_t conn_cli_pkg_sn(conn_cli_pkg_t pkg);
void conn_cli_pkg_set_sn(conn_cli_pkg_t pkg, uint32_t sn);

dp_req_t conn_cli_pkg_to_dp_req(conn_cli_pkg_t pkg);
conn_cli_pkg_t conn_cli_pkg_find(dp_req_t pkg);

#ifdef __cplusplus
}
#endif

#endif
