#ifndef GD_DR_STORE_H
#define GD_DR_STORE_H
#include "cpe/utils/buffer.h"
#include "net_trans_types.h"

#ifdef __cplusplus
extern "C" {
#endif

net_trans_task_t net_trans_task_create(net_trans_group_t group, size_t capacity);
void net_trans_task_free(net_trans_task_t task);

net_trans_task_state_t net_trans_task_state(net_trans_task_t task);
net_trans_task_result_t net_trans_task_result(net_trans_task_t task);

void * net_trans_task_data(net_trans_task_t task);
size_t net_trans_task_data_capacity(net_trans_task_t task);

mem_buffer_t net_trans_task_buffer(net_trans_task_t task);

int net_trans_task_start(net_trans_task_t task);

int net_trans_task_set_post_to(net_trans_task_t task, const char * uri, const char * data, int data_len);
void net_trans_task_set_commit_op(net_trans_task_t task, net_trans_task_commit_op_t op, void * ctx);

int net_trans_task_set_ssl_cainfo(net_trans_task_t task, const char * ca_file);

const char * net_trans_task_state_str(net_trans_task_state_t state);
const char * net_trans_task_result_str(net_trans_task_result_t result);

#ifdef __cplusplus
}
#endif

#endif
