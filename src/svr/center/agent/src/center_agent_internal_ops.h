#ifndef SVR_CENTER_AGENT_INTERNAL_OPS_H
#define SVR_CENTER_AGENT_INTERNAL_OPS_H
#include "center_agent_internal_types.h"
#include "protocol/svr/center/svr_center_pro.h"

int center_agent_center_ep_init(center_agent_t agent, net_ep_t ep);

int center_agent_center_send(center_agent_t agent, SVR_CENTER_PKG * pkg, size_t pkg_size);

typedef void (*center_agent_center_op_t)(center_agent_t agent, SVR_CENTER_PKG * pkg, size_t pkg_size);

#endif
