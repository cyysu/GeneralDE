#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_request.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "rank_g_svr_ops.h"

void rank_g_svr_request_remove(rank_g_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    SVR_RANK_G_REQ_REMOVE * req;
    char buf[svr->m_record_size];
    ptr_int_t record_idx;
    void * record;
    uint16_t index_pos;

    req = &((SVR_RANK_G_PKG*)dp_req_data(pkg_body))->data.svr_rank_g_req_remove;

    if (dr_entry_set_from_uint64(buf + svr->m_uin_start_pos, req->user_id, svr->m_uin_entry, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: request remove: set uin to data data fail!", rank_g_svr_name(svr));
        rank_g_svr_send_error_response(svr, pkg_head, SVR_RANK_G_ERROR_INTERNAL);
        return;
    }

    record = aom_obj_hash_table_find(svr->m_record_hash, buf);
    if (record == NULL) {
        CPE_INFO(svr->m_em, "%s: request remove: record of "FMT_UINT64_T" not exist!", rank_g_svr_name(svr), req->user_id);
        rank_g_svr_send_error_response(svr, pkg_head, SVR_RANK_G_ERROR_RECORD_NOT_EXIST);
        return;
    }
    record_idx = aom_obj_index(svr->m_record_mgr, record);

    for(index_pos = 0; index_pos < svr->m_index_count; ++index_pos) {
        rank_g_svr_index_t index = svr->m_indexs + index_pos;
        if (index->m_svr == NULL) continue;

        rank_g_svr_index_remove(index, (uint32_t)record_idx);
    }

    if (set_pkg_sn(pkg_head)) {
        if (set_svr_stub_reply_cmd(svr->m_stub, pkg_body, SVR_RANK_G_CMD_RES_REMOVE) != 0) {
            CPE_ERROR(svr->m_em, "%s: request remove: send response fail!", rank_g_svr_name(svr));
            return;
        }
    }
}
