#ifndef GD_NET_TRANS_INTERNAL_OPS_H
#define GD_NET_TRANS_INTERNAL_OPS_H
#include "net_trans_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*mgr ops*/
int net_trans_mult_handler_init(net_trans_manage_t svr);

/*group ops*/
uint32_t net_trans_group_hash(net_trans_group_t group);
int net_trans_group_eq(net_trans_group_t l, net_trans_group_t r);
void net_trans_group_free_all(net_trans_manage_t mgr);

/*task ops*/
int net_trans_task_set_done(net_trans_task_t task, net_trans_task_result_t result);

uint32_t net_trans_task_hash(net_trans_task_t task);
int net_trans_task_eq(net_trans_task_t l, net_trans_task_t r);

#ifdef __cplusplus
}
#endif

#endif

