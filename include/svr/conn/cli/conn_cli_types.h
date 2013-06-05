#ifndef SVR_CONN_CLI_TYPES_H
#define SVR_CONN_CLI_TYPES_H
#include "cpe/pal/pal_types.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct conn_cli * conn_cli_t;
typedef struct conn_cli_svr_stub  * conn_cli_svr_stub_t;
typedef struct conn_cli_pkg * conn_cli_pkg_t;

typedef enum conn_cli_state {
    conn_cli_state_disable
    , conn_cli_state_disconnected
    , conn_cli_state_connecting
    , conn_cli_state_established
} conn_cli_state_t;

#ifdef __cplusplus
}
#endif

#endif
