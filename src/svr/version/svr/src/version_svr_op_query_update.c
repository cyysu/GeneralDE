#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "version_svr_ops.h"

void version_svr_op_query_update(version_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body) {
    SVR_VERSION_REQ_QUERY_UPDATE const * req;
    SVR_VERSION_RES_QUERY_UPDATE * response;
    dp_req_t response_pkg;
    uint16_t update_version_count = 0;

    req = &((SVR_VERSION_PKG*)dp_req_data(pkg_body))->data.svr_version_req_query_update;

    response_pkg = set_svr_stub_outgoing_pkg_buf(svr->m_stub, sizeof(SVR_VERSION_PKG) + sizeof(SVR_VERSION_PACKAGE) * update_version_count);
    if (response_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: version_svr_op_query_update: get response pkg fail, size=%d!",
            version_svr_name(svr), (int)(sizeof(SVR_VERSION_PKG) + sizeof(SVR_VERSION_PACKAGE) * update_version_count));
        version_svr_send_error_response(svr, pkg_head, pkg_body, SVR_VERSION_ERRNO_INTERNAL);
        return;
    }

    response = set_svr_stub_pkg_to_data(svr->m_stub, response_pkg, 0, svr->m_meta_res_query_update, NULL);
    assert(response);

    response->app_id = req->app_id;
    response->version_min.parts[0] = 0;
    response->version_min.parts[1] = 0;
    response->version_min.parts[2] = 1;
    response->version_max.parts[0] = 0;
    response->version_max.parts[1] = 0;
    response->version_max.parts[2] = 1;
    response->update_strategy = SVR_VERSION_UPDATE_STRATEGY_NO;
    response->package_count = 0;

    if (set_svr_stub_reply_pkg(svr->m_stub, pkg_body, response_pkg) != 0) {
        CPE_ERROR(svr->m_em, "%s: version_svr_op_query: send response fail!", version_svr_name(svr));
        return;
    }
}
