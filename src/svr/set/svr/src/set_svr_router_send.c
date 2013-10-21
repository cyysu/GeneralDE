#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "set_svr_router_ops.h"
#include "protocol/svr/set/set_share_pkg.h"

int set_svr_router_send(set_svr_router_t router, dp_req_t body, dp_req_t head, dp_req_t carry, size_t * write_size) {
    set_svr_t svr = router->m_svr;
    size_t curent_pkg_size = 2048;
    ringbuffer_block_t blk;
    char * buf;
    int32_t encode_size;
    uint32_t pkg_size;
    SET_PKG_HEAD * head_buf;
    SET_PKG_HEAD * input_head_buf;

RESIZE_AND_TRY_AGAIN:
    blk = set_svr_ringbuffer_alloc(svr, curent_pkg_size, router->m_id);
    if (blk == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: router %d-%d.%d: not enouth ringbuf, curent_pkg_size=%d, data-len=%d!",
            set_svr_name(svr), router->m_id, router->m_ip, router->m_port, (int)curent_pkg_size, (int)pkg_size);
        set_svr_router_clear_data(router);
        return -1;
    }

    buf = NULL;
    ringbuffer_data(svr->m_ringbuf, blk, curent_pkg_size, 0, (void*)&buf);
    assert(buf);

    pkg_size = sizeof(uint32_t);

    /*写入包头*/
    head_buf = (void*)(buf + pkg_size);
    input_head_buf = dp_req_data(head);

    CPE_COPY_HTON16(&head_buf->to_svr_type, &input_head_buf->to_svr_type);
    CPE_COPY_HTON16(&head_buf->to_svr_id, &input_head_buf->to_svr_id);
    CPE_COPY_HTON16(&head_buf->from_svr_type, &input_head_buf->from_svr_type);
    CPE_COPY_HTON16(&head_buf->from_svr_id, &input_head_buf->from_svr_id);
    CPE_COPY_HTON32(&head_buf->sn, &input_head_buf->sn);
    CPE_COPY_HTON16(&head_buf->flags, &input_head_buf->flags);

    pkg_size += sizeof(*head_buf);

    /*写入carry_info*/
    memcpy(buf + pkg_size, dp_req_data(carry), dp_req_size(carry));
    pkg_size += dp_req_size(carry);

    /*写入包体*/
    encode_size =
        dr_pbuf_write(
            buf + pkg_size, curent_pkg_size - pkg_size,
            dp_req_data(body), dp_req_size(body), dp_req_meta(body), svr->m_em);
    if (encode_size < 0) {
        if (encode_size == dr_code_error_not_enough_output) {
            if (curent_pkg_size >= svr->m_router_max_pkg_size) {
                CPE_ERROR(
                    svr->m_em, "%s: router %d-%d.%d: send: not enough encode buf, buf size is %d!",
                    set_svr_name(svr), router->m_id, router->m_ip, router->m_port,
                    (int)curent_pkg_size);

                blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
                assert(blk == NULL);

                return -1;
            }
            else {
                curent_pkg_size *= 2;
                if (curent_pkg_size > svr->m_router_max_pkg_size) curent_pkg_size = svr->m_router_max_pkg_size;

                blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
                assert(blk == NULL);

                goto RESIZE_AND_TRY_AGAIN;
            }
        }
        else {
            CPE_ERROR(
                router->m_svr->m_em, "%s: router %d-%d.%d: send: encode fail, buf size is %d!",
                set_svr_name(router->m_svr), router->m_id, router->m_ip, router->m_port,
                (int)curent_pkg_size);

            blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
            assert(blk == NULL);

            return -1;
        }
    }

    pkg_size += encode_size;

    CPE_COPY_HTON32(buf, &pkg_size);

    ringbuffer_shrink(router->m_svr->m_ringbuf, blk, pkg_size);
    set_svr_router_link_node_w(router, blk);

    if (write_size) *write_size = pkg_size;

    if (router->m_svr->m_debug) {
        CPE_INFO(
            router->m_svr->m_em, "%s: router %d-%d.%d: ==> send one request, (net-size=%d)\n\thead: %s\tcarry: %s\tbody: %s",
            set_svr_name(router->m_svr), router->m_id, router->m_ip, router->m_port,
            (int)pkg_size,
            dp_req_dump(head, &svr->m_dump_buffer_head),
            dp_req_dump(carry, &svr->m_dump_buffer_carry),
            dp_req_dump(body, &svr->m_dump_buffer_body));
    }

    if (router->m_conn) {
        set_svr_router_conn_apply_evt(router->m_conn, set_svr_router_conn_fsm_evt_wb_update);
    }

    return 0;
}
