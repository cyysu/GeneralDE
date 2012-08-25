#ifndef USF_MONGO_DRIVER_INTERNAL_OPS_H
#define USF_MONGO_DRIVER_INTERNAL_OPS_H
#include "mongo_internal_types.h"

/*source info ops*/
struct mongo_source_info *
mongo_source_info_create(
    mongo_driver_t driver,
    int32_t source,
    const char * incoming_dispatch_to,
    const char * outgoing_recv_at);

uint32_t mongo_source_info_hash(const struct mongo_source_info * binding);
int mongo_source_info_eq(const struct mongo_source_info * l, const struct mongo_source_info * r);
void mongo_source_info_free_all(mongo_driver_t driver);
struct mongo_source_info * mongo_source_info_find(mongo_driver_t driver, int32_t source);

/*process ops*/
int mongo_driver_send(dp_req_t req, void * ctx, error_monitor_t em);
void mongo_driver_recv(net_ep_t ep, void * ctx, net_ep_event_t event);

void mongo_driver_process_connect(mongo_driver_t driver, mongo_pkg_t pkg);

#endif
