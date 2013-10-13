#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "cmcc_deliver_svr_ops.h"
#include "protocol/svr/cmcc_deliver/svr_cmcc_deliver_pro.h"

static cmcc_deliver_svr_op_t g_svr_cmcc_deliver_request_ops[] = {
    NULL
    , cmcc_deliver_svr_op_deliver, NULL
};

int cmcc_deliver_svr_request_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    cmcc_deliver_svr_t svr = ctx;
    dp_req_t pkg_head;
    SVR_CMCC_DELIVER_PKG * pkg;

    pkg_head = set_pkg_head_find(req);
    if (pkg_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: process request: no pkg head!", cmcc_deliver_svr_name(svr));
        return -1;
    }

    pkg = dp_req_data(req);

    if (pkg->cmd >= (sizeof(g_svr_cmcc_deliver_request_ops) / sizeof(g_svr_cmcc_deliver_request_ops[0]))
        || g_svr_cmcc_deliver_request_ops[pkg->cmd] == NULL)
    {
        CPE_ERROR(svr->m_em, "%s: process request: not support cmd %d!", cmcc_deliver_svr_name(svr), pkg->cmd);
        return -1;
    }
    
    g_svr_cmcc_deliver_request_ops[pkg->cmd](svr, req, pkg_head);

    return 0;
}
