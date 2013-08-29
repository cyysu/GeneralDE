#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "conn_svr_ops.h"

void conn_svr_op_bind_user(conn_svr_t svr, dp_req_t agent_pkg) {
    SVR_CONN_REQ_BIND_USER * req;
    conn_svr_conn_t conn = NULL;

    req = &((SVR_CONN_PKG*)dp_req_data(agent_pkg))->data.svr_conn_req_bind_user;

    conn = conn_svr_conn_find_by_conn_id(svr, req->conn_id);
    if (conn == NULL) {
        CPE_ERROR(svr->m_em, "%s: bind_user: conn %d not exist!", conn_svr_name(svr), req->conn_id);
        return;
    }

    if (conn_svr_conn_set_user_id(conn, req->user_id) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: bind_user: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T") set user "FMT_UINT64_T" fail!",
            conn_svr_name(svr), conn->m_conn_id, conn->m_fd, conn->m_user_id, req->user_id);
        return;
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: bind_user: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T") set user "FMT_UINT64_T" success!",
            conn_svr_name(svr), conn->m_conn_id, conn->m_fd, conn->m_user_id, req->user_id);
    }
}
