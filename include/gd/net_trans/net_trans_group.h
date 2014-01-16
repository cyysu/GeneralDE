#ifndef GD_NET_TRANS_GROUP_H
#define GD_NET_TRANS_GROUP_H
#include "net_trans_types.h"

#ifdef __cplusplus
extern "C" {
#endif

net_trans_group_t net_trans_group_create(net_trans_manage_t, const char * name);
void net_trans_group_free(net_trans_group_t group);

void net_trans_group_set_transfer_timeout(net_trans_group_t group, uint64_t timeout_ms);
void net_trans_group_set_connect_timeout(net_trans_group_t group, uint64_t timeout_ms);

net_trans_group_t net_trans_group_find(net_trans_manage_t, const char * name);

#ifdef __cplusplus
}
#endif

#endif
