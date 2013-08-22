#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/net/net_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/share/set_chanel.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/stub/set_svr_cmd_info.h"
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
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id);
        return -1;
    }

    carry = set_pkg_carry_find(body);

    head_buf = dp_req_data(head);

    if (head_buf->from_svr_type == 0) head_buf->from_svr_type = stub->m_svr_type->m_svr_type_id;
    if (head_buf->from_svr_id == 0) head_buf->from_svr_id = stub->m_svr_id;

    /*检查源地址*/
    if (head_buf->from_svr_type != stub->m_svr_type->m_svr_type_id || head_buf->from_svr_id != stub->m_svr_id) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d: from_svr %d.%d mismatch!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            head_buf->to_svr_type, head_buf->to_svr_id,
            head_buf->from_svr_type, head_buf->from_svr_id);
        return -1;
    }

    /*根据包类型获取包所属的服务类型*/
    switch(set_pkg_category(head)) {
    case set_pkg_request: {
        set_svr_svr_info_t to_svr_type;
        to_svr_type = set_svr_svr_info_find(stub, head_buf->to_svr_type);
        if (to_svr_type == NULL) {
            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: send one pkg to %d.%d: to svr type %d is unknown!",
                set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                head_buf->to_svr_type, head_buf->to_svr_id, head_buf->to_svr_type);
            return -1;
        }
        pkg_meta = to_svr_type->m_pkg_meta;
        break;
    }
    case set_pkg_response:
    case set_pkg_notify:
        pkg_meta = stub->m_svr_type->m_pkg_meta;
        break;
    default:
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d: pkg category %d is unknown!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
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
                set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                head_buf->to_svr_type, head_buf->to_svr_id,
                dp_req_meta(body) ? dr_meta_name(dp_req_meta(body)) : "???", dp_req_meta(body),
                dr_meta_name(pkg_meta), pkg_meta);
            return -1;
        }
    }
    else {
        CPE_ERROR(stub->m_em, "xxxxxx: pkg_meta=%s", dr_meta_name(pkg_meta));
        dp_req_set_meta(body, pkg_meta);
    }

    if ((rv = set_chanel_w_write(stub->m_chanel, body, &send_size)) != 0) {
        if (set_pkg_pack_state(head) == set_pkg_packed) {
            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d: send fail, error=%d (%s)!\nhead: %s\ncarry: %s\nbody: [packed %d bytes]",
                set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                head_buf->to_svr_id, head_buf->to_svr_id,
                rv, set_chanel_str_error(rv),
                dp_req_dump(head, &stub->m_dump_buffer_head),
                carry ? dp_req_dump(carry, &stub->m_dump_buffer_carry) : "none",
                (int)dp_req_size(body));
        }
        else {
            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d: send fail, error=%d (%s)!\nhead: %s\ncarry: %s\nbody: %s",
                set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
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
                stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                head_buf->to_svr_type, head_buf->to_svr_id, (int)send_size,
                dp_req_dump(head, &stub->m_dump_buffer_head),
                carry ? dp_req_dump(carry, &stub->m_dump_buffer_carry) : "none",
                (int)dp_req_size(body));
        }
        else {
            CPE_INFO(
                stub->m_em, "%s: svr %s.%d: ==> send one pkg to %d.%d (size=%d):\n\thead: %s\tcarry: %s\tbody: %s",
                set_svr_stub_name(stub),
                stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                head_buf->to_svr_type, head_buf->to_svr_id, (int)send_size,
                dp_req_dump(head, &stub->m_dump_buffer_head),
                carry ? dp_req_dump(carry, &stub->m_dump_buffer_carry) : "none",
                dp_req_dump(body, &stub->m_dump_buffer_body));
        }
    }

    return 0;
}

int set_svr_stub_send_req_data(
    set_svr_stub_t stub, uint16_t to_svr_type_id, uint16_t to_svr_id,
    uint16_t sn, void const * data, uint16_t data_size, LPDRMETA meta)
{
    dp_req_t pkg;
    dp_req_t pkg_head;
    set_svr_svr_info_t to_svr_info;
    set_svr_cmd_info_t cmd_info;
    size_t total_size;

    to_svr_info = set_svr_svr_info_find(stub, to_svr_type_id);
    if (to_svr_info == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send %s to %d.%d: target svr not exist!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            dr_meta_name(meta), to_svr_type_id, to_svr_id);
        return -1;
    }

    if (to_svr_info->m_pkg_data_entry == NULL || to_svr_info->m_pkg_cmd_entry == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send %s to %d.%d: no pkg cmd info!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            dr_meta_name(meta), to_svr_type_id, to_svr_id);
        return -1;
    }

    cmd_info = set_svr_cmd_info_find_by_name(stub, to_svr_info, dr_meta_name(meta));
    if (cmd_info == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send %s to %d.%d: cmd is unknown!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            dr_meta_name(meta), to_svr_type_id, to_svr_id);
        return -1;
    }

    total_size = (size_t)data_size + dr_entry_data_start_pos(to_svr_info->m_pkg_data_entry, 0);
    pkg = set_svr_stub_outgoing_pkg_buf(stub, total_size);

    dp_req_set_size(pkg, total_size);

    memcmp(
        ((char*)dp_req_data(pkg)) + dr_entry_data_start_pos(to_svr_info->m_pkg_data_entry, 0),
        data,
        data_size);

    if (dr_entry_set_from_uint32(
            ((char*)dp_req_data(pkg)) + dr_entry_data_start_pos(to_svr_info->m_pkg_cmd_entry, 0),
            (uint32_t)dr_entry_id(cmd_info->m_entry), to_svr_info->m_pkg_cmd_entry, stub->m_em) != 0)
    {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send %s to %d.%d: set cmd fail!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            dr_meta_name(meta), to_svr_type_id, to_svr_id);
        return -1;
    }

    pkg_head = set_pkg_head_check_create(pkg);
    if (pkg_head == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send %s to %d.%d: create pkg head fail!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            dr_meta_name(meta), to_svr_type_id, to_svr_id);
        return -1;
    }

    set_pkg_set_sn(pkg_head, sn);
    set_pkg_set_to_svr(pkg_head, to_svr_type_id, to_svr_id);
    set_pkg_set_category(pkg_head, set_pkg_request);

    return set_svr_stub_send_pkg(stub, pkg);
}

int set_svr_stub_send_req_cmd(
    set_svr_stub_t stub, uint16_t to_svr_type_id, uint16_t to_svr_id,
    uint16_t sn, uint32_t cmd)
{
    dp_req_t pkg;
    dp_req_t pkg_head;
    set_svr_svr_info_t to_svr_info;

    to_svr_info = set_svr_svr_info_find(stub, to_svr_type_id);
    if (to_svr_info == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send cmd %d to %d.%d: target svr not exist!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            cmd, to_svr_type_id, to_svr_id);
        return -1;
    }

    assert(to_svr_info->m_pkg_meta);

    if (to_svr_info->m_pkg_data_entry == NULL || to_svr_info->m_pkg_cmd_entry == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send cmd %d to %d.%d: no pkg cmd info!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            cmd, to_svr_type_id, to_svr_id);
        return -1;
    }

    pkg = set_svr_stub_outgoing_pkg_buf(stub, (size_t)dr_meta_size(to_svr_info->m_pkg_meta));
    dp_req_set_size(pkg, dr_meta_size(to_svr_info->m_pkg_meta));

    if (dr_entry_set_from_uint32(
            ((char*)dp_req_data(pkg)) + dr_entry_data_start_pos(to_svr_info->m_pkg_cmd_entry, 0),
            cmd, to_svr_info->m_pkg_cmd_entry, stub->m_em) != 0)
    {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send cmd %d to %d.%d: set cmd fail!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            cmd, to_svr_type_id, to_svr_id);
        return -1;
    }

    pkg_head = set_pkg_head_check_create(pkg);
    if (pkg_head == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send cmd %d to %d.%d: create pkg head fail!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            cmd, to_svr_type_id, to_svr_id);
        return -1;
    }

    set_pkg_set_sn(pkg_head, sn);
    set_pkg_set_to_svr(pkg_head, to_svr_type_id, to_svr_id);
    set_pkg_set_category(pkg_head, set_pkg_request);

    return set_svr_stub_send_pkg(stub, pkg);
}

int set_svr_stub_send_response_data(
    set_svr_stub_t stub, uint16_t to_svr_type_id, uint16_t to_svr_id,
    uint16_t sn, void const * data, uint16_t data_size, LPDRMETA meta)
{
    dp_req_t pkg;
    dp_req_t pkg_head;
    set_svr_cmd_info_t cmd_info;
    size_t total_size;

    if (stub->m_svr_type->m_pkg_data_entry == NULL || stub->m_svr_type->m_pkg_cmd_entry == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send response %s to %d.%d: no pkg cmd info!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            dr_meta_name(meta), to_svr_type_id, to_svr_id);
        return -1;
    }

    cmd_info = set_svr_cmd_info_find_by_name(stub, stub->m_svr_type, dr_meta_name(meta));
    if (cmd_info == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send response %s to %d.%d: cmd is unknown!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            dr_meta_name(meta), to_svr_type_id, to_svr_id);
        return -1;
    }

    total_size = (size_t)data_size + dr_entry_data_start_pos(stub->m_svr_type->m_pkg_data_entry, 0);
    pkg = set_svr_stub_outgoing_pkg_buf(stub, total_size);

    dp_req_set_size(pkg, total_size);

    memcmp(
        ((char*)dp_req_data(pkg)) + dr_entry_data_start_pos(stub->m_svr_type->m_pkg_data_entry, 0),
        data,
        data_size);

    if (dr_entry_set_from_uint32(
            ((char*)dp_req_data(pkg)) + dr_entry_data_start_pos(stub->m_svr_type->m_pkg_cmd_entry, 0),
            (uint32_t)dr_entry_id(cmd_info->m_entry), stub->m_svr_type->m_pkg_cmd_entry, stub->m_em) != 0)
    {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send response %s to %d.%d: set cmd fail!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            dr_meta_name(meta), to_svr_type_id, to_svr_id);
        return -1;
    }

    pkg_head = set_pkg_head_check_create(pkg);
    if (pkg_head == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: svr %s.%d: send response %s to %d.%d: create pkg head fail!",
            set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
            dr_meta_name(meta), to_svr_type_id, to_svr_id);
        return -1;
    }

    set_pkg_set_sn(pkg_head, sn);
    set_pkg_set_to_svr(pkg_head, to_svr_type_id, to_svr_id);
    set_pkg_set_category(pkg_head, set_pkg_response);

    return set_svr_stub_send_pkg(stub, pkg);
}

