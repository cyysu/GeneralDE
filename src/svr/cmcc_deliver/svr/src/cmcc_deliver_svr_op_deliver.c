#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "cmcc_deliver_svr_ops.h"
#include "protocol/svr/cmcc_deliver/svr_cmcc_deliver_pro.h"

void cmcc_deliver_svr_op_deliver(cmcc_deliver_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    SVR_CMCC_DELIVER_RES_DELIVER res;

    strncpy(res.transIDO, "aaa", sizeof(res.transIDO));
    res.hRet = 0;

    if (set_svr_stub_send_response_data(
            svr->m_stub,
            set_pkg_from_svr_type(pkg_head), set_pkg_from_svr_id(pkg_head), set_pkg_sn(pkg_head),
            &res, sizeof(res), svr->m_pkg_meta_res_deliver,
            NULL, 0)
        != 0)
    {
        CPE_ERROR(svr->m_em, "%s: op deliver: send response fail!", cmcc_deliver_svr_name(svr));
        return;
    }
}
