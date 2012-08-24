#ifndef USF_MONGO_DRIVER_INTERNAL_OPS_H
#define USF_MONGO_DRIVER_INTERNAL_OPS_H
#include "mongo_internal_types.h"

/**/
void mongo_driver_ep_process(net_ep_t ep, void * ctx, net_ep_event_t event);

#endif
