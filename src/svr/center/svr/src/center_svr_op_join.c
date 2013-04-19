#include <assert.h> 
#include "cpe/net/net_endpoint.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "center_svr_ops.h"

void center_cli_conn_op_join(center_cli_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size) {
    center_svr_t svr = conn->m_svr;
    SVR_CENTER_REQ_JOIN * req = &pkg->data.svr_center_req_join;
    center_cli_data_t data;
    SVR_CENTER_PKG * res = center_svr_get_res_pkg_buff(svr, pkg, sizeof(SVR_CENTER_PKG));
    if (res == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: ep %d: join: alloc res buf fail!",
            center_svr_name(svr), center_cli_conn_id(conn));
        return;
    }

    data = center_cli_data_find(svr, req->id.svr_type, req->id.svr_id);
    if (data == NULL) {
        SVR_CENTER_CLI_RECORD * record = aom_obj_alloc(svr->m_client_data_mgr);
        if (record == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: ep %d: join: alloc SVR_CENTER_CLI_RECORD fail!",
                center_svr_name(svr), center_cli_conn_id(conn));
            return;
        }

        record->svr_type = req->id.svr_type;
        record->svr_id = req->id.svr_id;
        record->port = req->port;
        record->online_time = 0;

        if (net_ep_peername(conn->m_ep, &record->ip, NULL) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: ep %d: join: get peer ip fail!",
                center_svr_name(svr), center_cli_conn_id(conn));
            aom_obj_free(svr->m_client_data_mgr, record);
            return;
        }

        data = center_cli_data_create(svr, record);
        if (data == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: ep %d: join: create cli data fail!",
                center_svr_name(svr), center_cli_conn_id(conn));
            aom_obj_free(svr->m_client_data_mgr, record);
            return;
        }
    }

    if (data->m_conn != conn) {
        if (data->m_conn) {
            CPE_INFO(
                svr->m_em, "%s: ep %d: join: bind to svr %d.%d, close old conn %d!",
                center_svr_name(svr), center_cli_conn_id(conn),
                data->m_data->svr_type, data->m_data->svr_id,
                center_cli_conn_id(data->m_conn));

            assert(data->m_conn->m_data == data);
            center_cli_conn_free(data->m_conn);
        }
        assert(data->m_conn == NULL);

        if (conn->m_data && conn->m_data != data) {
            CPE_INFO(
                svr->m_em, "%s: ep %d: join: bind to svr %d.%d, clear old svr %d.%d!",
                center_svr_name(svr), center_cli_conn_id(conn),
                data->m_data->svr_type, data->m_data->svr_id,
                conn->m_data->m_data->svr_type, conn->m_data->m_data->svr_id);

            center_cli_data_free(conn->m_data);
        }
        assert(conn->m_data == NULL);

        conn->m_data = data;
        data->m_conn = conn;
    }

    res->cmd = SVR_CENTER_CMD_RES_JOIN;
    res->sn = pkg->sn;
    res->data.svr_center_res_join.result = 0;

    center_cli_conn_send(conn, res, sizeof(*res));
}
