#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/share/set_chanel.h"
#include "set_svr_router_ops.h"
#include "protocol/svr/set/set_share_pkg.h"

static int set_svr_dispatch_send_pkg(set_svr_t svr, set_svr_svr_t local_svr, dp_req_t body, dp_req_t head, dp_req_t carry);
static set_svr_svr_t set_svr_dispatch_select_target(set_svr_svr_type_t to_svr_type, dp_req_t body, dp_req_t head);
 
ptr_int_t set_svr_dispatch_tick(void * ctx, ptr_int_t arg) {
    set_svr_t svr = ctx;
    ptr_int_t process_count = 0;
    set_svr_svr_t first_svr = NULL;;
    dp_req_t body;
    dp_req_t head;
    dp_req_t carry;
    int rv;

    if (svr->m_incoming_buf == NULL) {
        svr->m_incoming_buf = dp_req_create(gd_app_dp_mgr(svr->m_app), 0);
        if (svr->m_incoming_buf == NULL) {
            CPE_ERROR(svr->m_em, "%s: dispatch: alloc incoming pkg body fail!", set_svr_name(svr));
            return 0;
        }
    }

    body = svr->m_incoming_buf;

    if ((head = set_pkg_head_check_create(body)) == NULL) {
        CPE_ERROR(svr->m_em, "%s: dispatch: alloc incoming pkg head fail!", set_svr_name(svr));
        dp_req_free(svr->m_incoming_buf);
        svr->m_incoming_buf = NULL;
        return 0;
    }

    if ((carry = set_pkg_carry_check_create(body, 0)) == NULL) {
        CPE_ERROR(svr->m_em, "%s: dispatch: alloc incoming pkg carry fail!", set_svr_name(svr));
        dp_req_free(svr->m_incoming_buf);
        svr->m_incoming_buf = NULL;
        return 0;
    }
    
    for(process_count = 0; process_count < svr->m_router_process_count_per_tick;) {
        set_svr_svr_t local_svr;

        local_svr = TAILQ_FIRST(&svr->m_local_svrs);
        if (local_svr == NULL) {
            if (svr->m_debug >= 2) {
                CPE_INFO(svr->m_em, "%s: dispatch: no local svr!", set_svr_name(svr));
            }
            break;
        }

        if (first_svr == NULL) first_svr = local_svr;
        else if (first_svr == local_svr) break; 

        TAILQ_REMOVE(&svr->m_local_svrs, local_svr, m_next_for_local);
        TAILQ_INSERT_TAIL(&svr->m_local_svrs, local_svr, m_next_for_local);

        if (local_svr->m_chanel == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: dispatch: svr %s.%d: no chanel!",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id);
            continue;
        }

        rv = set_chanel_w_peak(local_svr->m_chanel, body);
        if (rv != 0) {
            if (rv == set_chanel_error_chanel_empty) continue;

            CPE_ERROR(
                svr->m_em, "%s: dispatch: svr %s.%d: peak pkg fail!",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id);
            continue;
        }

        ++process_count;

        if (set_svr_dispatch_send_pkg(svr, local_svr, body, head, carry) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: dispatch: svr %s.%d: peak pkg fail!",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id);
            set_chanel_w_erase(local_svr->m_chanel);
            continue;
        }

        set_chanel_w_erase(local_svr->m_chanel);
    }

    if (svr->m_debug >= 3) {
        CPE_INFO(svr->m_em, "%s: dispatch: process %d pkg!", set_svr_name(svr), (int)process_count);
    }

    return process_count;
}

static set_svr_svr_t set_svr_dispatch_select_target(set_svr_svr_type_t to_svr_type, dp_req_t body, dp_req_t head) {
    return NULL;
}

static int set_svr_dispatch_send_pkg(set_svr_t svr, set_svr_svr_t local_svr, dp_req_t body, dp_req_t head, dp_req_t carry) {
    SET_PKG_HEAD * head_buf = dp_req_data(head);
    set_svr_svr_type_t to_svr_type;
    set_svr_svr_t to_svr;
    size_t write_size;
    LPDRMETA pkg_meta;

    to_svr_type = set_svr_svr_type_find_by_id(svr, head_buf->to_svr_type);
    if (to_svr_type == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: dispatch: svr %s.%d: target svr type %d is unknown!",
            set_svr_name(svr),
            local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
            head_buf->to_svr_type);
        return -1;
    }

    if (head_buf->to_svr_id != 0) {
        to_svr = set_svr_svr_find(svr, head_buf->to_svr_type, head_buf->to_svr_id);
        if (to_svr == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: dispatch: svr %s.%d: target svr %s.%d not exist!",
                set_svr_name(svr),
                local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                to_svr_type->m_svr_type_name, head_buf->to_svr_id);
            return -1;
        }
    }
    else {
        to_svr = set_svr_dispatch_select_target(to_svr_type, body, head);
        if (to_svr == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: dispatch: svr %s.%d: auto select target %s.??? fail!",
                set_svr_name(svr),
                local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                to_svr_type->m_svr_type_name);
            return -1;
        }
    }

    assert(to_svr);

    pkg_meta = set_svr_get_pkg_meta(svr, head, to_svr_type, NULL);
    if (pkg_meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: dispatch: svr %s.%d: get pkg meta fail!",
            set_svr_name(svr),
            local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id);
        return -1;
    }
    dp_req_set_meta(body, pkg_meta);

    if (to_svr->m_category == set_svr_svr_local) { /*发送到本地服务 */
        int rv;

        if (to_svr->m_chanel == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: dispatch: svr %s.%d: to svr %s.%d no chanel!",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                to_svr->m_svr_type->m_svr_type_name, to_svr->m_svr_id);
            return -1;
        }

        rv = set_chanel_r_write(to_svr->m_chanel, body, &write_size);
        if (rv != 0) {
            CPE_ERROR(
                svr->m_em, "%s: dispatch: svr %s.%d: write to chanel of svr %s.%d fail, error=%d (%s)!",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                to_svr->m_svr_type->m_svr_type_name, to_svr->m_svr_id, rv, set_chanel_str_error(rv));
            return -1;
        }

        if (svr->m_debug >= 2) {
            CPE_INFO(
                svr->m_em, "%s: dispatch: svr %s.%d: write one pkg to local svr %s.%d (size=%d)\n\thead: %s\tcarry: %s\tbody: %s",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                to_svr->m_svr_type->m_svr_type_name, to_svr->m_svr_id,
                (int)write_size,
                dp_req_dump(head, &svr->m_dump_buffer_head),
                dp_req_dump(carry, &svr->m_dump_buffer_carry),
                dp_req_dump(body, &svr->m_dump_buffer_body));
        }
        else if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: dispatch: svr %s.%d: write one pkg to local svr %s.%d (size=%d)",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                to_svr->m_svr_type->m_svr_type_name, to_svr->m_svr_id, (int)write_size);
        }
    }
    else {
        if (to_svr->m_router == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: dispatch: svr %s.%d: to svr %s.%d: no route!",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                to_svr->m_svr_type->m_svr_type_name, to_svr->m_svr_id);
            return -1;
        }

        if (set_svr_router_send(to_svr->m_router, body, head, carry, &write_size) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: dispatch: svr %s.%d: write one pkg to remote svr %s.%d by router %d-%d.%d fail!",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                to_svr->m_svr_type->m_svr_type_name, to_svr->m_svr_id,
                to_svr->m_router->m_id, to_svr->m_router->m_ip, to_svr->m_router->m_port);
            return -1;
        }

        if (svr->m_debug >= 2) {
            CPE_INFO(
                svr->m_em, "%s: dispatch: svr %s.%d: write one pkg to remote svr %s.%d by router %d-%d.%d (size=%d)\n\thead: %s\tcarry: %s\tbody: %s",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                to_svr->m_svr_type->m_svr_type_name, to_svr->m_svr_id,
                to_svr->m_router->m_id, to_svr->m_router->m_ip, to_svr->m_router->m_port,
                (int)write_size,
                dp_req_dump(head, &svr->m_dump_buffer_head),
                dp_req_dump(carry, &svr->m_dump_buffer_carry), 
                dp_req_dump(body, &svr->m_dump_buffer_body));
        }
        else if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: dispatch: svr %s.%d: write one pkg to remote svr %s.%d by router %d-%d.%d (size=%d)",
                set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                to_svr->m_svr_type->m_svr_type_name, to_svr->m_svr_id,
                to_svr->m_router->m_id, to_svr->m_router->m_ip, to_svr->m_router->m_port,
                (int)write_size);
        }

        if (to_svr->m_router->m_conn == NULL) {
            set_svr_router_conn_t conn = set_svr_router_conn_create(svr, to_svr->m_router, -1);
            if (conn == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: dispatch: svr %s.%d: to svr %s.%d: create conn fail!",
                    set_svr_name(svr), local_svr->m_svr_type->m_svr_type_name, local_svr->m_svr_id,
                    to_svr->m_svr_type->m_svr_type_name, to_svr->m_svr_id);
                return -1;
            }
            assert(to_svr->m_router->m_conn == conn);
        }
    }

    return 0;
}
