#include <assert.h> 
#include "center_svr_ops.h"
#include "protocol/svr/center/svr_center_pro.h"

void center_svr_conn_op_query_by_type(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size) {
    center_svr_t svr = conn->m_svr;
    SVR_CENTER_REQ_QUERY_SVR_BY_TYPE * req = &pkg->data.svr_center_req_query_svr_by_type;
    uint16_t type_i;
    size_t record_count = 0;
    SVR_CENTER_PKG * res;
    size_t res_len;
    SVR_CENTER_RES_QUERY_SVR_BY_TYPE * res_records;

    for(type_i = 0; type_i < req->count; ++type_i) {
        center_cli_group_t group = center_cli_group_find(svr, req->types[type_i]);
        if (group) record_count += group->m_svr_count;
    }

    res_len = sizeof(SVR_CENTER_PKG) + record_count * sizeof(SVR_CENTER_SVR_INFO);
    res = center_svr_get_res_pkg_buff(svr, pkg, res_len);
    if (res == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: query by type: alloc res buf fail!",
            center_svr_name(svr), conn->m_fd);
        return;
    }

    res->cmd = SVR_CENTER_CMD_RES_QUERY_SVR_BY_TYPE;

    res_records = &res->data.svr_center_res_query_svr_by_type;

    res_records->count = 0;
    for(type_i = 0; type_i < req->count; ++type_i) {
        center_cli_data_t data;
        center_cli_group_t group;

        group = center_cli_group_find(svr, req->types[type_i]);
        if (group == NULL) continue;

        TAILQ_FOREACH(data, &group->m_datas, m_next) {
            SVR_CENTER_SVR_INFO * res_record = &res_records->data[res_records->count++];

            res_record->id.svr_type = data->m_data->svr_type;
            res_record->id.svr_id = data->m_data->svr_id;
            res_record->ip = data->m_data->ip;
            res_record->port = data->m_data->port;
        }
    }

    center_svr_conn_send(conn, res, res_len);
}
