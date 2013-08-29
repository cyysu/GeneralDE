#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "match_svr_ops.h"

static match_svr_op_t g_svr_match_request_ops[] = {
    NULL
    , match_svr_request_join, NULL
    , match_svr_request_leave, NULL
};

int match_svr_match_require_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    match_svr_t svr = ctx;
    dp_req_t pkg_head;
    SVR_MATCH_PKG * pkg;

    pkg_head = set_pkg_head_find(req);
    if (pkg_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: process match request: no pkg head!", match_svr_name(svr));
        return -1;
    }

    pkg = dp_req_data(req);

    if (pkg->cmd >= (sizeof(g_svr_match_request_ops) / sizeof(g_svr_match_request_ops[0]))
        || g_svr_match_request_ops[pkg->cmd] == NULL)
    {
        CPE_ERROR(svr->m_em, "%s: process match request: not support cmd %d!", match_svr_name(svr), pkg->cmd);
        return -1;
    }
    
    g_svr_match_request_ops[pkg->cmd](svr, req, pkg_head);

    return 0;
}
