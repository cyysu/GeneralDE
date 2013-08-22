#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "room_svr_ops.h"

void room_svr_op_broadcast(room_svr_t svr, dp_req_t agent_pkg) {
    SVR_ROOM_REQ_BROADCAST * req;
    room_svr_room_t room = NULL;
    room_svr_user_t user = NULL;
    uint32_t cur_time = room_svr_cur_time(svr);

    req = &((SVR_ROOM_PKG*)dp_req_data(agent_pkg))->data.svr_room_req_broadcast;

    room = room_svr_room_find(svr, req->room_id);
    if (room == NULL) {
        CPE_ERROR(svr->m_em, "%s: broadcast: room "FMT_UINT64_T" not exist!", room_svr_name(svr), req->room_id);
        return;
    }

    user = room_svr_room_lsearch_user(room, req->user_id);
    if (user == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: broadcast: room "FMT_UINT64_T": user "FMT_UINT64_T" not exist in room!",
            room_svr_name(svr), req->room_id, req->user_id);
        return;
    }

    user->m_data->user_at_svr_type = set_pkg_from_svr_type(agent_pkg);
    user->m_data->user_at_svr_id = set_pkg_from_svr_id(agent_pkg);

    room_svr_user_update_state(user, cur_time, SVR_ROOM_USER_ACTIVE);

    room_svr_room_notify_msg(room, req->user_id, req->data, req->data_len);
    
    return;
}
