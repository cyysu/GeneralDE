#ifndef SVR_RANK_G_SVR_OPS_H
#define SVR_RANK_G_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "rank_g_svr_types.h"
#include "protocol/svr/rank_g/svr_rank_g_pro.h"

#ifdef __cplusplus
extern "C" {
#endif

/*operations of rank_g_svr */
rank_g_svr_t
rank_g_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em);

void rank_g_svr_free(rank_g_svr_t svr);

rank_g_svr_t rank_g_svr_find(gd_app_context_t app, cpe_hash_string_t name);
rank_g_svr_t rank_g_svr_find_nc(gd_app_context_t app, const char * name);
const char * rank_g_svr_name(rank_g_svr_t svr);
uint32_t rank_g_svr_cur_time(rank_g_svr_t svr);

int rank_g_svr_set_request_recv_at(rank_g_svr_t svr, const char * name);
int rank_g_svr_set_check_span(rank_g_svr_t svr, uint32_t span_ms);

int rank_g_svr_record_init(rank_g_svr_t svr, uint32_t record_count, float bucket_ratio);

/*index*/
rank_g_svr_index_t
rank_g_svr_index_create(rank_g_svr_t svr, uint16_t id, const char * entry_path);
rank_g_svr_index_t rank_g_svr_index_find(rank_g_svr_t svr, uint16_t id);

int rank_g_svr_index_update(rank_g_svr_index_t index, uint32_t record_id);
void rank_g_svr_index_remove(rank_g_svr_index_t index, uint32_t record_id);

int rank_g_svr_index_query(
    rank_g_svr_t svr, rank_g_svr_index_t index,
    SVR_RANK_G_QUERY const * query, uint32_t require_count,
    int (*visit_fun)(void * ctx, void const * record), void * visit_ctx);

/*rank_g request ops*/
void rank_g_svr_request_update(rank_g_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);
void rank_g_svr_request_remove(rank_g_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);
void rank_g_svr_request_query(rank_g_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);
void rank_g_svr_request_query_with_data(rank_g_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

/*protocol utils*/
void rank_g_svr_send_error_response(rank_g_svr_t svr, dp_req_t pkg_head, int16_t error);

#ifdef __cplusplus
}
#endif

#endif
