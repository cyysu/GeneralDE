#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "room_svr_ops.h"

void room_svr_op_broadcast(room_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body) {
    SVR_ROOM_REQ_BROADCAST * req;
    room_svr_room_t room = NULL;

    req = &((SVR_ROOM_PKG*)dp_req_data(pkg_body))->data.svr_room_req_broadcast;

    room = room_svr_room_find(svr, req->room_id);
    if (room == NULL) {
        CPE_ERROR(svr->m_em, "%s: broadcast: room "FMT_UINT64_T" not exist!", room_svr_name(svr), req->room_id);

        SVR_ROOM_NOTIFY_ROOM_DESTORY pkg;
        pkg.room_id = req->room_id;
        pkg.user_id = req->user_id;
        pkg.destory_reason = SVR_ROOM_ROOM_DESTORY_TIMEOUT;

        if (set_svr_stub_send_notify_data(
                svr->m_stub, set_pkg_from_svr_type(pkg_head), set_pkg_from_svr_id(pkg_head), 0,
                &pkg, sizeof(pkg), svr->m_pkg_meta_notify_room_destoried, NULL, 0)
            != 0)
        {
            CPE_ERROR(svr->m_em, "%s: broadcast: send notify_room_destoried fail!", room_svr_name(svr));
            return;
        }

        return;
    }

    if (room->m_logic_svr && set_pkg_from_svr_type(pkg_head) == set_svr_svr_info_svr_type_id(room->m_logic_svr)) {
        /*从逻辑服务过来的包 */
        room_svr_room_notify_msg(room, req->user_id, req->data_type, req->data, req->data_len);
        return;
    }
    else {
        /*从用户过来的包 */
        room_svr_user_t user = NULL;

        user = room_svr_room_lsearch_user(room, req->user_id);
        if (user == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: broadcast: room "FMT_UINT64_T": user "FMT_UINT64_T" not exist in room!",
                room_svr_name(svr), req->room_id, req->user_id);
            return;
        }

        room_svr_user_update_state(user, room_svr_cur_time(svr), SVR_ROOM_USER_ACTIVE);

        if (room->m_logic_svr) {
            room_svr_p_notify_room_message(svr, room->m_logic_svr, req);
        }
        else {
            room_svr_room_notify_msg(room, req->user_id, req->data_type, req->data, req->data_len);
        }
    }
    
    return;
}
