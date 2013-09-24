#include <assert.h> 
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/base64.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/net_trans/net_trans_task.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "apple_iap_svr_ops.h"

static void apple_iap_svr_request_validate_commit(net_trans_task_t task, void * ctx);

static void apple_iap_svr_request_validate_send_error_response(
    apple_iap_svr_t svr, const char * receipt, int error,
    uint32_t sn, uint16_t from_svr_type, uint16_t from_svr_id);

void apple_iap_svr_request_validate(apple_iap_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    net_trans_task_t task;
    SVR_APPLE_IAP_REQ_VALIDATE * req;
    struct apple_iap_task_data * task_data;
    char request_buf[512];
    struct write_stream_mem request_stream = CPE_WRITE_STREAM_MEM_INITIALIZER(request_buf, sizeof(request_buf));
    struct read_stream_mem receipt_stream;

    req = &((SVR_APPLE_IAP_PKG*)dp_req_data(pkg_body))->data.svr_apple_iap_req_validate;

    task = net_trans_task_create(svr->m_trans_group, sizeof(struct apple_iap_task_data));
    if (task == NULL) {
        CPE_ERROR(svr->m_em, "%s: validate: create task fail!", apple_iap_svr_name(svr));

        apple_iap_svr_request_validate_send_error_response(
            svr, req->receipt, -1,
            set_pkg_sn(pkg_head), set_pkg_from_svr_type(pkg_head), set_pkg_from_svr_id(pkg_head));

        return;
    }

    task_data = net_trans_task_data(task);

    strncpy(task_data->m_receipt, req->receipt, sizeof(task_data->m_receipt));
    task_data->m_sn = set_pkg_sn(pkg_head);
    task_data->m_from_svr_type = set_pkg_from_svr_type(pkg_head);
    task_data->m_from_svr_id = set_pkg_from_svr_id(pkg_head);

    net_trans_task_set_commit_op(task, apple_iap_svr_request_validate_commit, svr);

    /*构造请求 */
    read_stream_mem_init(&receipt_stream, req->receipt, strlen(req->receipt));

    stream_printf((write_stream_t)&request_stream, "{\"receipt-data\":");
    cpe_base64_encode((write_stream_t)&request_stream, (read_stream_t)&receipt_stream);
    stream_printf((write_stream_t)&request_stream, "}");
    stream_putc((write_stream_t)&request_stream, 0);

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: validate %s: send request %s!", apple_iap_svr_name(svr), task_data->m_receipt, request_buf);
    }

    /*发送请求 */
    if (net_trans_task_set_post_to(
            task, 
            "https://sandbox.itunes.apple.com/verifyReceipt",
            request_buf,
            123)
        != 0)
    {
        CPE_ERROR(svr->m_em, "%s: validate %s: post %s fail!", apple_iap_svr_name(svr), req->receipt, request_buf);
        net_trans_task_free(task);
        return;
    }

    if (net_trans_task_start(task) != 0) {
        CPE_ERROR(svr->m_em, "%s: validate %s: start request fail!", apple_iap_svr_name(svr), req->receipt);
        net_trans_task_free(task);
        return;
    }

    return;
}

static void apple_iap_svr_request_validate_commit(net_trans_task_t task, void * ctx) {
    apple_iap_svr_t svr = ctx;
    struct apple_iap_task_data * task_data = net_trans_task_data(task);
    SVR_APPLE_IAP_RES_VALIDATE res;

    if (net_trans_task_result(task) != net_trans_result_ok) {
        CPE_ERROR(
            svr->m_em, "%s: validate %s: task not ok, result is %s!",
            apple_iap_svr_name(svr), task_data->m_receipt,
            net_trans_task_result_str(net_trans_task_result(task)));

        apple_iap_svr_request_validate_send_error_response(
            svr, task_data->m_receipt, -1,
            task_data->m_sn, task_data->m_from_svr_type, task_data->m_from_svr_id);

        return;
    }

    bzero(&res, sizeof(res));
    if (dr_json_read(
            &res, sizeof(res),
            mem_buffer_make_continuous(net_trans_task_buffer(task), 0),
            svr->m_meta_res_validate,
            svr->m_em)
        < 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: validate %s: parse response fail\n%s!",
            apple_iap_svr_name(svr), task_data->m_receipt,
            mem_buffer_make_continuous(net_trans_task_buffer(task), 0));

        apple_iap_svr_request_validate_send_error_response(
            svr, task_data->m_receipt, -1,
            task_data->m_sn, task_data->m_from_svr_type, task_data->m_from_svr_id);

        return;
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: validate %s: receive response\n%s!",
            apple_iap_svr_name(svr), task_data->m_receipt,
            mem_buffer_make_continuous(net_trans_task_buffer(task), 0));
    }

    if (set_svr_stub_send_response_data(
            svr->m_stub, task_data->m_from_svr_type, task_data->m_from_svr_id, task_data->m_sn,
            &res, sizeof(res), svr->m_meta_res_validate, NULL, 0)
        != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: validate %s: send response error!",
            apple_iap_svr_name(svr), task_data->m_receipt);
        return;
    }
}

static void apple_iap_svr_request_validate_send_error_response(
    apple_iap_svr_t svr, const char * receipt, int error,
    uint32_t sn, uint16_t from_svr_type, uint16_t from_svr_id)
{
    SVR_APPLE_IAP_RES_ERROR res;
    res.error = error;

    if (set_svr_stub_send_response_data(
            svr->m_stub, from_svr_type, from_svr_id, sn,
            &res, sizeof(res), svr->m_meta_res_error, NULL, 0)
        != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: validate %s: send error response error!",
            apple_iap_svr_name(svr), receipt);
        return;
    }
}
