#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/net/net_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/center/agent/center_agent_svr_type.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/share/set_chanel.h"
#include "svr/set/stub/set_svr_stub.h"
#include "set_svr_stub_internal_ops.h"
#include "protocol/svr/set/set_share_pkg.h"

int set_svr_stub_outgoing_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    return set_svr_stub_send_pkg(ctx, req);
}

int set_svr_stub_send_pkg(set_svr_stub_t stub, dp_req_t body) {
    dp_req_t head;
    dp_req_t carry;
    LPDRMETA pkg_meta;
    SET_PKG_HEAD * head_buf;
    size_t send_size;
    int rv;

    head = set_pkg_head_find(body);
    if (head == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send one pkg, no head info!",
            set_svr_stub_name(stub), center_agent_svr_type_name(stub->m_svr_type), stub->m_svr_id);
        return -1;
    }

    carry = set_pkg_carry_find(body);

    head_buf = dp_req_data(head);

    if (head_buf->from_svr_type == 0) head_buf->from_svr_type = center_agent_svr_type_id(stub->m_svr_type);
    if (head_buf->from_svr_id == 0) head_buf->from_svr_id = stub->m_svr_id;

    /*检查源地址*/
    if (head_buf->from_svr_type != center_agent_svr_type_id(stub->m_svr_type) || head_buf->from_svr_id != stub->m_svr_id) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d: from_svr %d.%d mismatch!",
            set_svr_stub_name(stub), center_agent_svr_type_name(stub->m_svr_type), stub->m_svr_id,
            head_buf->to_svr_type, head_buf->to_svr_id,
            head_buf->from_svr_type, head_buf->from_svr_id);
        return -1;
    }

    /*根据包类型获取包所属的服务类型*/
    switch(set_pkg_category(head)) {
    case set_pkg_request: {
        center_agent_svr_type_t to_svr_type;
        to_svr_type = center_agent_svr_type_find(stub->m_agent, head_buf->to_svr_type);
        if (to_svr_type == NULL) {
            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: send one pkg to %d.%d: to svr type %d is unknown!",
                set_svr_stub_name(stub), center_agent_svr_type_name(stub->m_svr_type), stub->m_svr_id,
                head_buf->to_svr_type, head_buf->to_svr_id, head_buf->to_svr_type);
            return -1;
        }
        pkg_meta = center_agent_svr_type_pkg_meta(to_svr_type);
        break;
    }
    case set_pkg_response:
    case set_pkg_notify:
        pkg_meta = center_agent_svr_type_pkg_meta(stub->m_svr_type);
        break;
    default:
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d: pkg category %d is unknown!",
            set_svr_stub_name(stub), center_agent_svr_type_name(stub->m_svr_type), stub->m_svr_id,
            head_buf->to_svr_type, head_buf->to_svr_id,
            (int)set_pkg_category(head));
        return -1;
    }

    assert(pkg_meta);

    /*检查发送数据类型*/
    if (dp_req_meta(body)) {
        if (pkg_meta && dp_req_meta(body) != pkg_meta) {
            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d: pkg meta mismatch, %s(%p) and %s(%p)!",
                set_svr_stub_name(stub), center_agent_svr_type_name(stub->m_svr_type), stub->m_svr_id,
                head_buf->to_svr_type, head_buf->to_svr_id,
                dp_req_meta(body) ? dr_meta_name(dp_req_meta(body)) : "???", dp_req_meta(body),
                dr_meta_name(pkg_meta), pkg_meta);
            return -1;
        }
    }
    else {
        dp_req_set_meta(body, pkg_meta);
    }

    if ((rv = set_chanel_w_write(stub->m_chanel, body, &send_size)) != 0) {
        if (set_pkg_pack_state(head) == set_pkg_packed) {
            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d: send fail, error=%d (%s)!\nhead: %s\ncarry: %s\nbody: [packed %d bytes]",
                set_svr_stub_name(stub), center_agent_svr_type_name(stub->m_svr_type), stub->m_svr_id,
                head_buf->to_svr_id, head_buf->to_svr_id,
                rv, set_chanel_str_error(rv),
                dp_req_dump(head, &stub->m_dump_buffer_head),
                carry ? dp_req_dump(carry, &stub->m_dump_buffer_carry) : "none",
                (int)dp_req_size(body));
        }
        else {
            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d: send fail, error=%d (%s)!\nhead: %s\ncarry: %s\nbody: %s",
                set_svr_stub_name(stub), center_agent_svr_type_name(stub->m_svr_type), stub->m_svr_id,
                head_buf->to_svr_id, head_buf->to_svr_id,
                rv, set_chanel_str_error(rv),
                dp_req_dump(head, &stub->m_dump_buffer_head),
                carry ? dp_req_dump(carry, &stub->m_dump_buffer_carry) : "none",
                dp_req_dump(body, &stub->m_dump_buffer_body));
        }
        return -1;
    }

    if (stub->m_debug) {
        if (set_pkg_pack_state(head) == set_pkg_packed) {
            CPE_INFO(
                stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d (size=%d):\n\thead: %s\tcarry: %s\tbody: [packed %d bytes]",
                set_svr_stub_name(stub),
                center_agent_svr_type_name(stub->m_svr_type), stub->m_svr_id,
                head_buf->to_svr_type, head_buf->to_svr_id, (int)send_size,
                dp_req_dump(head, &stub->m_dump_buffer_head),
                carry ? dp_req_dump(carry, &stub->m_dump_buffer_carry) : "none",
                (int)dp_req_size(body));
        }
        else {
            CPE_INFO(
                stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d (size=%d):\n\thead: %s\tcarry: %s\tbody: %s",
                set_svr_stub_name(stub),
                center_agent_svr_type_name(stub->m_svr_type), stub->m_svr_id,
                head_buf->to_svr_type, head_buf->to_svr_id, (int)send_size,
                dp_req_dump(head, &stub->m_dump_buffer_head),
                carry ? dp_req_dump(carry, &stub->m_dump_buffer_carry) : "none",
                dp_req_dump(body, &stub->m_dump_buffer_body));
        }
    }

    return 0;
}
