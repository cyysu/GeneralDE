#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "rank_f_svr_ops.h"

void rank_f_svr_request_query(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    SVR_RANK_F_REQ_QUERY * req;
    rank_f_svr_index_t index;
    uint16_t gid_start_pos;
    dp_req_t response_body;
    SVR_RANK_F_RES_QUERY * res;
    rank_f_svr_index_buf_t buf;
    int rv;

    req = &((SVR_RANK_F_PKG*)dp_req_data(pkg_body))->data.svr_rank_f_req_query;

    rv = rank_f_svr_user_index_check_create(&index, svr, req->user_id, req->index_id);
    if (rv != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request query: check create index "FMT_UINT64_T".%d fail",
            rank_f_svr_name(svr), req->user_id, req->index_id);
        rank_f_svr_send_error_response(svr, pkg_head, rv);
        return;
    }

    res = rank_f_svr_make_response(&response_body, svr, pkg_body, sizeof(uint64_t) * index->m_record_count);
    if (res == NULL) {
        CPE_ERROR(svr->m_em, "%s: request query: make response fail!", rank_f_svr_name(svr));
        rank_f_svr_send_error_response(svr, pkg_head, rv);
        return;
    }

    gid_start_pos = rank_f_svr_gid_start_pos(svr);

    res->user_id = req->user_id;
    res->index_id = req->index_id;
    res->user_id_count = 0;
    for(buf = index->m_bufs; buf; buf = buf->m_next) {
        uint8_t i;
        for(i = 0;
            i < RANK_F_SVR_INDEX_BUF_RECORD_COUNT && res->user_id_count < index->m_record_count;
            ++i, ++res->user_id_count)
        {
            res->user_ids[i] = *(uint64_t *)(((char const *)(buf->m_records[i] + 1)) + gid_start_pos);
        }
    }

    if (set_svr_stub_send_pkg(svr->m_stub, response_body) != 0) {
        CPE_ERROR(svr->m_em, "%s: request query: send response fail!", rank_f_svr_name(svr));
    }
}
