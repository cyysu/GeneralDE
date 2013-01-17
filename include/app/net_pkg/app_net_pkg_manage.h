#ifndef APP_NET_PKG_MANAGE_H
#define APP_NET_PKG_MANAGE_H
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "app_net_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

app_net_pkg_manage_t
app_net_pkg_manage_create(
    gd_app_context_t app,
    const char * name,
    error_monitor_t em);

void app_net_pkg_manage_free(app_net_pkg_manage_t mgr);

app_net_pkg_manage_t
app_net_pkg_manage_find(gd_app_context_t app, cpe_hash_string_t name);

app_net_pkg_manage_t
app_net_pkg_manage_find_nc(gd_app_context_t app, const char * name);

app_net_pkg_manage_t
app_net_pkg_manage_default(gd_app_context_t app);

gd_app_context_t app_net_pkg_manage_app(app_net_pkg_manage_t mgr);
const char * app_net_pkg_manage_name(app_net_pkg_manage_t mgr);
cpe_hash_string_t app_net_pkg_manage_name_hs(app_net_pkg_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif
