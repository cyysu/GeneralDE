#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "rank_g_svr_ops.h"

struct rank_g_svr_query_work_ctx {
    rank_g_svr_t svr;
    SVR_RANK_G_RES_QUERY * res;
};

static int rank_g_svr_build_query_result(void * ctx, void const * record) {
    return 0;
}

void rank_g_svr_request_query(rank_g_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    SVR_RANK_G_REQ_QUERY * req;
    rank_g_svr_index_t index;
    dp_req_t response_body;
    struct rank_g_svr_query_work_ctx work_ctx;
    int rv;

    req = &((SVR_RANK_G_PKG*)dp_req_data(pkg_body))->data.svr_rank_g_req_query;

    index = rank_g_svr_index_find(svr, req->index_id);
    if (index == NULL) {
        rank_g_svr_send_error_response(svr, pkg_head, SVR_RANK_G_ERROR_INDEX_NOT_EXIST);
        return;
    }

    response_body = set_svr_stub_outgoing_pkg_buf(svr->m_stub, sizeof(SVR_RANK_G_PKG) + sizeof(uint64_t) * req->require_count);

    work_ctx.svr = svr;
    work_ctx.res = set_svr_stub_pkg_to_data(svr->m_stub, response_body, 0, svr->m_pkg_meta_res_query, NULL);
    if (work_ctx.res == NULL) {
        CPE_ERROR(svr->m_em, "%s: request query: make response fail!", rank_g_svr_name(svr));
        rank_g_svr_send_error_response(svr, pkg_head, SVR_RANK_G_ERROR_INTERNAL);
        return;
    }

    work_ctx.res->index_id = req->index_id;
    work_ctx.res->query = req->query;
    work_ctx.res->user_id_count = 0;

    if ((rv = rank_g_svr_index_query(
             svr, index, 
             &req->query, req->require_count,
             rank_g_svr_build_query_result, &work_ctx)))
    {
        CPE_ERROR(svr->m_em, "%s: request query: build response error, rv=%d!", rank_g_svr_name(svr), rv);
        rank_g_svr_send_error_response(svr, pkg_head, rv);
        return;
    }

    if (set_svr_stub_reply_pkg(svr->m_stub, pkg_body, response_body) != 0) {
        CPE_ERROR(svr->m_em, "%s: request query: send response fail!", rank_g_svr_name(svr));
    }
}
