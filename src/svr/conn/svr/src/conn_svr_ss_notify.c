#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "svr/set/share/set_pkg.h"
#include "conn_svr_ops.h"

int conn_svr_ss_notify_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    conn_svr_t svr = ctx;
    dp_req_t req_head;
    conn_svr_backend_t backend;
    uint64_t user_id;
    conn_svr_conn_t conn;

    req_head = set_pkg_head_find(req);
    if (req_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: notify: find head fail!", conn_svr_name(svr));
        return -1;
    }

    backend = conn_svr_backend_find(svr, set_pkg_from_svr_type(req_head));
    if (backend == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: notify(svr_type=%d, svr_id=%d): backend not exist!",
            conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head));
        return -1;
    }

    if (backend->m_user_id_entry == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: notify(svr_type=%d, svr_id=%d): backend no user_id entry!",
            conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head));
        return -1;
    }

    /*TODO: 数据是没有decode的，需要修改为根据entry_id进行读取*/
    if (dr_entry_try_read_uint64(
            &user_id,
            ((char*)dp_req_data(req)) + backend->m_user_id_start,
            backend->m_user_id_entry, svr->m_em) != 0)
    {
        char buf[128];
        CPE_ERROR(
            svr->m_em, "%s: notify(svr_type=%d, svr_id=%d): read user id fail, start=%d, entry=%s(%s)!",
            conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head),
            (int)backend->m_user_id_start, dr_entry_name(backend->m_user_id_entry),
            dr_meta_off_to_path(backend->m_pkg_meta, (int)backend->m_user_id_start, buf, sizeof(buf)));
        return -1;
    }

    conn = conn_svr_conn_find_by_user_id(svr, user_id);
    if (conn == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: notify(svr_type=%d, svr_id=%d): no connection bind with user "FMT_UINT64_T"!",
            conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head),
            user_id);
        return -1;
    }

    if (conn_svr_conn_net_send(conn, set_pkg_from_svr_type(req_head), 0, 0, dp_req_data(req), dp_req_size(req), dp_req_meta(req)) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: notify(svr_type=%d, svr_id=%d): send data at conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T") fail!",
            conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head),
            conn->m_conn_id, conn->m_fd, conn->m_user_id);
        return -1;
    }
    
    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): send one notify(svr_type=%d, svr_id=%d)!",
            conn_svr_name(svr), conn->m_conn_id, conn->m_fd, conn->m_user_id,
            set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head));
    }

    return 0;
}
