#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "room_svr_ops.h"

static room_svr_op_t g_svr_ops[] = {
    NULL
    , room_svr_op_create, NULL
    , room_svr_op_delete, NULL
    , room_svr_op_query_by_type, NULL
    , room_svr_op_query_by_user, NULL
    , room_svr_op_join, NULL
    , room_svr_op_leave, NULL
    , room_svr_op_broadcast, NULL
};

int room_svr_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    room_svr_t svr = ctx;
    SVR_ROOM_PKG * pkg;

    pkg = dp_req_data(req);

    if (pkg->cmd >= (sizeof(g_svr_ops) / sizeof(g_svr_ops[0]))
        || g_svr_ops[pkg->cmd] == NULL)
    {
        CPE_ERROR(svr->m_em, "%s: process: not support cmd %d!", room_svr_name(svr), pkg->cmd);
        return -1;
    }
    
    g_svr_ops[pkg->cmd](svr, req);

    return 0;
}
