#include <assert.h>
#include "cpe/aom/aom_obj_mgr.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_rank_tree.h"

static int rank_g_svr_index_query_by_pos(
    rank_g_svr_t svr, rank_g_svr_index_t index,
    uint32_t start_pos, uint32_t require_count,
    int (*visit_fun)(void * ctx, void const * record), void * visit_ctx)
{
    rt_node_t rank_node;
    int r;

    assert(index);

    r = rt_find_by_rank(index->m_rank_tree, start_pos, &rank_node);
    if (r != 0) {
        CPE_ERROR(
            svr->m_em, "%s: query_by_pos: query by pos %d error, error=%d!",
            rank_g_svr_name(svr), start_pos, r);
        return r;
    }

    while(rank_node) {
        void * record = aom_obj_get(svr->m_record_mgr, rt_node_record_id(rank_node));
        assert(record);
        visit_fun(visit_ctx, record);
        --require_count;

        rank_node = rt_next(index->m_rank_tree, rank_node);
    }

    return 0;
}

int rank_g_svr_index_query(
    rank_g_svr_t svr, rank_g_svr_index_t index,
    SVR_RANK_G_QUERY const * query, uint32_t require_count,
    int (*visit_fun)(void * ctx, void const * record), void * visit_ctx)
{
    switch(query->query_type) {
    case SVR_RANK_G_QUERY_TYPE_BY_POS:
        return rank_g_svr_index_query_by_pos(svr, index, query->query_data.by_pos.start_pos, require_count, visit_fun, visit_ctx);
    default:
        CPE_ERROR(svr->m_em, "%s: query: query type %d is unknoqn!", rank_g_svr_name(svr), query->query_type);
        return SVR_RANK_G_ERROR_QUERY_TYPE_UNKNOWN;
    }

    return 0;
}


