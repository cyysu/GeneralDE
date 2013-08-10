#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "gd/timer/timer_manage.h"
#include "svr/center/agent/center_agent.h"
#include "svr/set/share/set_pkg.h"
#include "room_svr_ops.h"

void room_svr_op_join(room_svr_t svr, dp_req_t agent_pkg) {
    dp_req_t res_pkg;
    SVR_ROOM_REQ_JOIN * req;
    SVR_ROOM_RES_JOIN * result;
    room_svr_room_t room = NULL;
    room_svr_user_t user = NULL;
    uint32_t cur_time = room_svr_cur_time(svr);
    int16_t rv = -1;

    req = &((SVR_ROOM_PKG*)dp_req_data(agent_pkg))->data.svr_room_req_join;

    room = room_svr_room_find(svr, req->room_id);
    if (room == NULL) {
        CPE_ERROR(svr->m_em, "%s: join: room "FMT_UINT64_T": room not exist!", room_svr_name(svr), req->room_id);
        rv = SVR_ROOM_ERROR_ROOM_NOT_EXIST;
        goto JOIN_ROOM_FAIL;
    }

    user = room_svr_room_lsearch_user(room, req->user.user_id);
    if (user == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: join: room "FMT_UINT64_T": user "FMT_UINT64_T" not exist in room!",
            room_svr_name(svr), req->room_id, req->user.user_id);
        rv = SVR_ROOM_ERROR_USER_NOT_IN_ROOM;
        goto JOIN_ROOM_FAIL;
    }
    
    user->m_data->user_at_svr_type = set_pkg_from_svr_type(agent_pkg);
    user->m_data->user_at_svr_id = set_pkg_from_svr_id(agent_pkg);
    user->m_data->user_join_time = cur_time;
    user->m_data->user_data_len = req->user.user_data_len;
    memcpy(user->m_data->user_data, req->user.user_data, req->user.user_data_len);

    room_svr_user_update_state(user, cur_time, SVR_ROOM_USER_ACTIVE);

    room_svr_room_notify_user_join(room, user);

    res_pkg = room_svr_build_response(svr, agent_pkg, sizeof(SVR_ROOM_PKG));
    if (res_pkg == NULL) return;
    result = &((SVR_ROOM_PKG *)dp_req_data(res_pkg))->data.svr_room_res_join;
    result->result = 0;
    room_svr_send_pkg(svr, res_pkg);

    return;

JOIN_ROOM_FAIL:
    res_pkg = room_svr_build_response(svr, agent_pkg, sizeof(SVR_ROOM_PKG));
    if (res_pkg == NULL) return;
    result = &((SVR_ROOM_PKG *)dp_req_data(res_pkg))->data.svr_room_res_join;
    result->result = rv;
    room_svr_send_pkg(svr, res_pkg);
}
