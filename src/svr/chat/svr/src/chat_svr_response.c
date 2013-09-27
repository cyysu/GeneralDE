#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "chat_svr_ops.h"

void chat_svr_send_error_response(chat_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body, int err) {
    dp_req_t response_pkg;
    SVR_CHAT_PKG * response;

    if (set_pkg_sn(pkg_head) == 0) return;

    response_pkg = chat_svr_pkg_buf(svr, sizeof(SVR_CHAT_PKG));
    if (response_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_error_response: get response pkg fail!", chat_svr_name(svr));
        return;
    }

    if (set_pkg_init_response(response_pkg, pkg_body) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_error_response: init response fail!", chat_svr_name(svr));
        return;
    }

    response = dp_req_data(response_pkg);
    response->cmd = SVR_CHAT_CMD_RES_ERROR;
    response->data.svr_chat_res_error.error = err;

    if (set_svr_stub_send_pkg(svr->m_stub, response_pkg) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_error_response: send fail!", chat_svr_name(svr));
        return;
    }
}
