#ifndef APP_NET_PROXY_TYPES_H
#define APP_NET_PROXY_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "app/net_pkg/app_net_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum app_net_ep_state {
    app_net_ep_state_init,
    app_net_ep_state_registing,
    app_net_ep_state_error,
    app_net_ep_state_ok,
} app_net_ep_state_t;

typedef struct app_net_ep * app_net_ep_t;
typedef struct app_net_proxy * app_net_proxy_t;

#ifdef __cplusplus
}
#endif

#endif
