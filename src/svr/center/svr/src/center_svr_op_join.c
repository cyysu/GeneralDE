#include <assert.h> 
#include "cpe/pal/pal_socket.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "center_svr_ops.h"

static int8_t center_svr_conn_op_join_sync_data(center_svr_t svr, center_svr_conn_t conn, SVR_CENTER_REQ_JOIN const * req);

void center_svr_conn_op_join(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size) {
    center_svr_t svr = conn->m_svr;
    SVR_CENTER_REQ_JOIN * req = &pkg->data.svr_center_req_join;
    SVR_CENTER_PKG * res;
    int8_t join_rv = 0;

    res = center_svr_get_res_pkg_buff(svr, pkg, sizeof(SVR_CENTER_PKG));
    if (res == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: join: alloc res buf fail!",
            center_svr_name(svr), conn->m_fd);
        join_rv = SVR_CENTER_ERROR_INTERNAL;
        goto SEND_RESPONSE;
    }

    join_rv = center_svr_conn_op_join_sync_data(svr, conn, req);

SEND_RESPONSE:
    res->cmd = SVR_CENTER_CMD_RES_JOIN;
    res->data.svr_center_res_join.result = join_rv;

    center_svr_conn_send(conn, res, sizeof(*res));
}

static int8_t center_svr_conn_op_join_sync_data(center_svr_t svr, center_svr_conn_t conn, SVR_CENTER_REQ_JOIN const * req) {
    uint16_t i;
    struct sockaddr_in addr;
    socklen_t len;

    for(i = 0; i < req->count; ++i) {
        center_cli_data_t data;
        SVR_CENTER_SVR_RUNING const * runing_svr;

        runing_svr = &req->data[i];
        
        data = center_cli_data_find(svr, runing_svr->id.svr_type, runing_svr->id.svr_id);
        if (data == NULL) {
            SVR_CENTER_CLI_RECORD * record;
            center_cli_group_t group;

            group = center_cli_group_find(svr, runing_svr->id.svr_type);
            if (group == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: join: group of %d not exist!",
                    center_svr_name(svr), conn->m_fd, runing_svr->id.svr_type);
                return SVR_CENTER_ERROR_SVR_TYPE_NOT_EXIST;
            }

            record = aom_obj_alloc(svr->m_client_data_mgr);
            if (record == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: join: alloc SVR_CENTER_CLI_RECORD fail!",
                    center_svr_name(svr), conn->m_fd);
                return SVR_CENTER_ERROR_INTERNAL;
            }

            record->svr_type = runing_svr->id.svr_type;
            record->svr_id = runing_svr->id.svr_id;
            record->port = runing_svr->port;
            record->online_time = 0;

            len = sizeof(addr);
            if (cpe_getpeername(conn->m_fd, (struct sockaddr *)&addr, &len) != 0) {
                CPE_ERROR(
                    svr->m_em,
                    "%s: conn %d: get perr ip fail, errno=%d (%s)!",
                    center_svr_name(svr), conn->m_fd, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
                aom_obj_free(svr->m_client_data_mgr, record);
                return SVR_CENTER_ERROR_INTERNAL;
            }
            record->ip = addr.sin_addr.s_addr;

            data = center_cli_data_create(svr, group, record);
            if (data == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: join: create cli data fail!",
                    center_svr_name(svr), conn->m_fd);
                aom_obj_free(svr->m_client_data_mgr, record);
                return SVR_CENTER_ERROR_INTERNAL;
            }
        }

        if (data->m_conn != conn) {
            if (data->m_conn) {
                CPE_INFO(
                    svr->m_em, "%s: conn %d: join: bind to svr %d.%d, close old conn %d!",
                    center_svr_name(svr), conn->m_fd,
                    data->m_data->svr_type, data->m_data->svr_id,
                    data->m_conn->m_fd);

                assert(data->m_conn->m_data == data);
                center_svr_conn_free(data->m_conn);
            }
            assert(data->m_conn == NULL);

            if (conn->m_data && conn->m_data != data) {
                CPE_INFO(
                    svr->m_em, "%s: conn %d: join: bind to svr %d.%d, clear old svr %d.%d!",
                    center_svr_name(svr), conn->m_fd,
                    data->m_data->svr_type, data->m_data->svr_id,
                    conn->m_data->m_data->svr_type, conn->m_data->m_data->svr_id);

                center_cli_data_free(conn->m_data);
            }
            assert(conn->m_data == NULL);

            conn->m_data = data;
            data->m_conn = conn;
        }
    }

    return 0;
}
