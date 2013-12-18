#include <assert.h> 
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_json.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "cpe/aom/aom_obj_hash.h"
#include "rank_g_svr_ops.h"

void rank_g_svr_request_update(rank_g_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    SVR_RANK_G_REQ_UPDATE * req;
    char buf[svr->m_record_size];
    int rv;
    ptr_int_t record_idx;
    uint16_t index_pos;

    req = &((SVR_RANK_G_PKG*)dp_req_data(pkg_body))->data.svr_rank_g_req_update;

    rv = dr_pbuf_read(buf, svr->m_record_size, req->data, req->data_len, svr->m_record_meta, svr->m_em);
    if (rv < 0) {
        CPE_ERROR(svr->m_em, "%s: request update: read data fail!", rank_g_svr_name(svr));
        rank_g_svr_send_error_response(svr, pkg_head, SVR_RANK_G_ERROR_RECORD_INPUT_DATA);
        return;
    }

    if (aom_obj_hash_table_insert_or_update(svr->m_record_hash, buf, &record_idx) != 0) {
        CPE_ERROR(svr->m_em, "%s: request update: create_or_update record fail!", rank_g_svr_name(svr));
        rank_g_svr_send_error_response(svr, pkg_head, SVR_RANK_G_ERROR_INTERNAL);
        return;
    }

    for(index_pos = 0; index_pos < svr->m_index_count; ++index_pos) {
        rank_g_svr_index_t index = svr->m_indexs + index_pos;
        if (index->m_svr == NULL) continue;

        rv = rank_g_svr_index_update(index, (uint32_t)record_idx);
        if (rv != 0) {
            CPE_ERROR(svr->m_em, "%s: request update: update record %d record fail!", rank_g_svr_name(svr), index_pos);
            rank_g_svr_send_error_response(svr, pkg_head, rv);
            return;
        }
    }

    if (set_pkg_sn(pkg_head)) {
        if (set_svr_stub_reply_cmd(svr->m_stub, pkg_body, SVR_RANK_G_CMD_RES_UPDATE) != 0) {
            CPE_ERROR(svr->m_em, "%s: request update: send response fail!", rank_g_svr_name(svr));
            return;
        }
    }
}
