#ifndef APP_NET_PKG_H
#define APP_NET_PKG_H
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "app_net_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern cpe_hash_string_t app_net_pkg_type_name;

app_net_pkg_t
app_net_pkg_create(
    app_net_pkg_manage_t mgr,
    size_t capacity);

void app_net_pkg_free(app_net_pkg_t pkg);

void app_net_pkg_init(app_net_pkg_t app_net_pkg);

uint16_t app_net_pkg_app_type(app_net_pkg_t req);

size_t app_net_pkg_data_capacity(app_net_pkg_t req);
void * app_net_pkg_data(app_net_pkg_t req);
size_t app_net_pkg_data_size(app_net_pkg_t req);
int app_net_pkg_data_set_size(app_net_pkg_t req, size_t size);

dp_req_t app_net_pkg_to_dp_req(app_net_pkg_t pkg);
app_net_pkg_t app_net_pkg_from_dp_req(dp_req_t pkg);

#ifdef __cplusplus
}
#endif

#endif
