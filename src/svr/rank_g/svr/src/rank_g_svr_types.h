#ifndef SVR_RANK_G_SVR_TYPES_H
#define SVR_RANK_G_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/aom/aom_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/rank_g/svr_rank_g_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rank_g_svr * rank_g_svr_t;
typedef struct rank_g_svr_index * rank_g_svr_index_t;
typedef struct rt * rt_t;
typedef struct rt_node * rt_node_t;

typedef int (*rank_g_svr_sort_fun_t)(void const * l, void const * r);

struct rank_g_svr_index {
    rank_g_svr_t m_svr;
    uint16_t m_id;
    LPDRMETAENTRY m_rank_entry;
    uint16_t m_rank_start_pos;
    uint32_t * m_record_to_rank_pos;
    rt_t m_rank_tree;
};

struct rank_g_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    LPDRMETA m_pkg_meta_res_error;
    LPDRMETA m_pkg_meta_res_query;
    LPDRMETA m_pkg_meta_res_query_with_data;

    gd_timer_id_t m_check_timer_id;

    dp_rsp_t m_recv_at;

    /*配置信息 */
    uint16_t m_index_count;
    struct rank_g_svr_index m_indexs[SVR_RANK_G_INDEX_MAX];

    /*record数据 */
    LPDRMETA m_record_meta;
    uint32_t m_record_size;
    LPDRMETAENTRY m_uin_entry;
    uint32_t m_uin_start_pos;
    aom_obj_mgr_t m_record_mgr;
    aom_obj_hash_table_t m_record_hash;
};

typedef void (*rank_g_svr_op_t)(rank_g_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

#ifdef __cplusplus
}
#endif

#endif
