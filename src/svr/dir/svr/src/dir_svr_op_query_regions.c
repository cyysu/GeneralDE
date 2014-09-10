#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "dir_svr_ops.h"

void dir_svr_op_query_regions(dir_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body) {
    SVR_DIR_REQ_QUERY_REGIONS const * req;
    SVR_DIR_RES_QUERY_REGIONS * response;
    dp_req_t response_pkg;
    dir_svr_region_t region;

    req = &((SVR_DIR_PKG*)dp_req_data(pkg_body))->data.svr_dir_req_query_regions;

    if (req->game_id != svr->m_game_id) {
        CPE_ERROR(
            svr->m_em, "%s: dir_svr_op_query_regions: game mismatch, input-game-id=%d, game-id=%d!",
            dir_svr_name(svr), req->game_id, svr->m_game_id);
        dir_svr_send_error_response(svr, pkg_head, pkg_body, SVR_DIR_ERRNO_GAME_MISMATCH);
        return;
    }

    response_pkg = set_svr_stub_outgoing_pkg_buf(svr->m_stub, sizeof(SVR_DIR_PKG) + sizeof(SVR_DIR_REGION) * svr->m_region_count);
    if (response_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: dir_svr_op_query_regions: get response pkg fail, size=%d!",
            dir_svr_name(svr), (int)(sizeof(SVR_DIR_PKG) + sizeof(SVR_DIR_REGION) * svr->m_region_count));
        dir_svr_send_error_response(svr, pkg_head, pkg_body, SVR_DIR_ERRNO_INTERNAL);
        return;
    }

    response = set_svr_stub_pkg_to_data(svr->m_stub, response_pkg, 0, svr->m_meta_res_query_regions, NULL);
    assert(response);

    response->region_count = 0;
    TAILQ_FOREACH(region, &svr->m_regions, m_next) {
        dir_svr_server_t server;
        SVR_DIR_REGION * o_region;

        server = TAILQ_FIRST(&region->m_servers);

        TAILQ_REMOVE(&region->m_servers, server, m_next);
        TAILQ_INSERT_TAIL(&region->m_servers, server, m_next);

        o_region = &response->regions[response->region_count++];
        o_region->region_id = region->m_region_id;
        strncpy(o_region->region_name, region->m_region_name, sizeof(o_region->region_name));

        if (server) {
            o_region->region_state = SVR_DIR_REGION_NORMAL;
            strncpy(o_region->suggest_server.ip, server->m_ip, sizeof(o_region->suggest_server.ip));
            o_region->suggest_server.port = server->m_port;
        }
        else {
            o_region->region_state = SVR_DIR_REGION_MAINTENANCE;
            o_region->suggest_server.ip[0] = 0;
            o_region->suggest_server.port = 0;
        }
    }

    if (set_svr_stub_reply_pkg(svr->m_stub, pkg_body, response_pkg) != 0) {
        CPE_ERROR(svr->m_em, "%s: dir_svr_op_query_regions: send response fail!", dir_svr_name(svr));
        return;
    }
}
