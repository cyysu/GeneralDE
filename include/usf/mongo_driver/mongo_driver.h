#ifndef USF_MONGO_DRIVER_H
#define USF_MONGO_DRIVER_H
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "mongo_driver_types.h"

#ifdef __cplusplus
extern "C" {
#endif

mongo_driver_t
mongo_driver_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void mongo_driver_free(mongo_driver_t driver);
int mongo_driver_build_reulst_metalib(mongo_driver_t driver);

mongo_driver_t
mongo_driver_find(gd_app_context_t app, cpe_hash_string_t name);

mongo_driver_t
mongo_driver_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t mongo_driver_app(mongo_driver_t driver);
const char * mongo_driver_name(mongo_driver_t driver);
cpe_hash_string_t mongo_driver_name_hs(mongo_driver_t driver);

mongo_driver_state_t mongo_driver_state(mongo_driver_t driver);

int mongo_driver_add_seed(mongo_driver_t driver, const char * host, int port);
int mongo_driver_add_server(mongo_driver_t driver, const char * host, int port);

int mongo_driver_add_dispatch(mongo_driver_t driver, const char * host, int port);

int mongo_driver_enable(mongo_driver_t driver);

mongo_pkg_t mongo_driver_req_buf(mongo_driver_t driver);

#ifdef __cplusplus
}
#endif

#endif
