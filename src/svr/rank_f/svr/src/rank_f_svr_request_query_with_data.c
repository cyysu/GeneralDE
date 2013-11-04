#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_pbuf.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "rank_f_svr_ops.h"

void rank_f_svr_request_query_with_data(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    SVR_RANK_F_REQ_QUERY_WITH_DATA * req;
    rank_f_svr_index_t index;
    dp_req_t response_body;
    SVR_RANK_F_RES_QUERY_WITH_DATA * res;
    rank_f_svr_index_buf_t buf;
    int rv;
    uint32_t data_buf_capacity;
    uint8_t * data_buf;

    req = &((SVR_RANK_F_PKG*)dp_req_data(pkg_body))->data.svr_rank_f_req_query_with_data;

    rv = rank_f_svr_user_index_check_create(&index, svr, req->user_id, req->index_id);
    if (rv != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request query-with-data: check create index "FMT_UINT64_T".%d fail",
            rank_f_svr_name(svr), req->user_id, req->index_id);
        rank_f_svr_send_error_response(svr, pkg_head, rv);
        return;
    }

    data_buf_capacity = svr->m_data_size * index->m_record_count;
    res = rank_f_svr_make_response(&response_body, svr, pkg_body, data_buf_capacity);
    if (res == NULL) {
        CPE_ERROR(svr->m_em, "%s: request query-with-data: make response fail!", rank_f_svr_name(svr));
        rank_f_svr_send_error_response(svr, pkg_head, rv);
        return;
    }

    res->user_id = req->user_id;
    res->index_id = req->index_id;
    res->start_pos = req->start_pos;
    res->total_count = index->m_record_count;
    res->return_count = 0;
    res->data_len = 0;
    data_buf = res->data;

    for(buf = index->m_bufs; buf; buf = buf->m_next) {
        uint8_t i = 0;

        if (req->start_pos) {
            if (req->start_pos >= RANK_F_SVR_INDEX_BUF_RECORD_COUNT) {
                req->start_pos -= RANK_F_SVR_INDEX_BUF_RECORD_COUNT;
                continue;
            }
            else {
                i = req->start_pos;
                req->start_pos = 0;
            }
        }

        for(; i < RANK_F_SVR_INDEX_BUF_RECORD_COUNT
                && data_buf_capacity > 0
                && res->return_count < req->require_count;
            ++i)
        {
            int size = dr_pbuf_write(
                data_buf, data_buf_capacity, 
                buf->m_records[i] + 1, svr->m_data_size, svr->m_data_meta, svr->m_em);
            if (size < 0) {
                CPE_ERROR(svr->m_em, "%s: request query-with-data: encode data fail!", rank_f_svr_name(svr));
                rank_f_svr_send_error_response(svr, pkg_head, -1);
                return;
            }

            assert(size > 0 && size <= data_buf_capacity);

            data_buf_capacity -= size;
            data_buf += size;
            res->return_count ++;
            res->data_len += size;
        }
    }

    if (set_svr_stub_send_pkg(svr->m_stub, response_body) != 0) {
        CPE_ERROR(svr->m_em, "%s: request query-with-data: send response fail!", rank_f_svr_name(svr));
    }
}
