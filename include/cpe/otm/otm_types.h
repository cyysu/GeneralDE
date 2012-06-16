#ifndef CPE_OTM_TYPES_H
#define CPE_OTM_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/tl/tl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t otm_timer_id_t;

typedef struct otm_memo_node {
    otm_timer_id_t m_id;
    tl_time_t m_last_action_time;
} * otm_memo_node_t;

typedef struct otm_timer * otm_timer_t;
typedef struct otm_manage * otm_manage_t;

typedef void (*otm_process_fun_t) (tl_time_t pre_exec_time, tl_time_t cur_exec_time, void * timer_ctx, void * obj_ctx);

#ifdef __cplusplus
}
#endif

#endif
