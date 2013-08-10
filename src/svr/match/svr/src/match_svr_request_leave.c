#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "gd/timer/timer_manage.h"
#include "svr/center/agent/center_agent.h"
#include "svr/set/share/set_pkg.h"
#include "match_svr_ops.h"

static void match_svr_process_creating_room(match_svr_t svr, match_svr_room_t creating_room, SVR_MATCH_ROOM_META const * room_meta);

void match_svr_request_leave(match_svr_t svr, dp_req_t agent_pkg) {
    dp_req_t res_pkg;
    SVR_MATCH_REQ_LEAVE * req;
    SVR_MATCH_RES_LEAVE * result;
    SVR_MATCH_ROOM_META * room_meta;
    match_svr_user_t user;
    match_svr_room_t room;
    int16_t err = SVR_MATCH_ERROR_INTERNAL;

    req = & ((SVR_MATCH_PKG *)dp_req_data(agent_pkg))->data.svr_match_req_leave;

    /*查找用户信息*/
    user = match_svr_user_find_in_room(svr, req->user_id, req->match_room_id);
    if (user == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: leave: user "FMT_UINT64_T" in room "FMT_UINT64_T" not exist!",
            match_svr_name(svr), req->user_id, req->match_room_id);
        err = SVR_MATCH_ERROR_USER_NOT_EXIST;
        goto REQ_LEAVE_MATCH_FAIL;
    }

    room = user->m_room;
    assert(room);

    /*获取房间配置数据*/
    room_meta = match_svr_meta_foom_find(svr, room->m_data->match_room_type);
    if (room_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: leave: match room type %d is unknown!", match_svr_name(svr), room->m_data->match_room_type);
        goto REQ_LEAVE_MATCH_FAIL;
    }

    /*清楚用户数据*/
    if (room_meta->sync_user) match_svr_room_notify_other_user_leave(room, user);
    match_svr_user_destory(user);

    /*发送成功响应*/
    res_pkg = match_svr_build_response(svr, agent_pkg, sizeof(SVR_MATCH_PKG));
    if (res_pkg == NULL) return;
    result = &((SVR_MATCH_PKG*)dp_req_data(res_pkg))->data.svr_match_res_leave;
    result->result = 0;
    match_svr_send_pkg(svr, res_pkg);

    if (room->m_data->creating_id > 0) { /*已经是正在创建房间的情况了*/
        match_svr_process_creating_room(svr, room, room_meta);
    }
    else {
        if (room->m_user_count == 0) match_svr_room_destory(room);
    }

    return;

REQ_LEAVE_MATCH_FAIL:
    /*发送失败响应*/
    res_pkg = match_svr_build_response(svr, agent_pkg, sizeof(SVR_MATCH_PKG));
    if (res_pkg == NULL) return;
    result = &((SVR_MATCH_PKG*)dp_req_data(res_pkg))->data.svr_match_res_leave;
    result->result = err;
    match_svr_send_pkg(svr, res_pkg);
}

static void match_svr_process_creating_room(match_svr_t svr, match_svr_room_t creating_room, SVR_MATCH_ROOM_META const * room_meta) {
    match_svr_room_t matching_room;

    matching_room = match_svr_room_find_matching(svr, creating_room->m_data->match_room_id);
    if (matching_room == NULL) { /*如果没有匹配队列，则直接将创建队列改回为匹配队列*/
        match_svr_room_to_matching(creating_room);
    }
    else if (matching_room->m_user_count == 0) { /*不应该到此，只是保护*/
        match_svr_room_destory(matching_room);
        match_svr_room_to_matching(creating_room);
    }
    else { /*将匹配队列中的第一个用户移动到创建队列*/
        match_svr_user_t move_user = TAILQ_FIRST(&matching_room->m_users);
        assert(move_user);

        match_svr_user_move_to_room(move_user, creating_room);
        match_svr_room_to_creating(creating_room); /*生成新的creating_id*/

        if (room_meta->sync_user) {
            match_svr_room_notify_other_user_leave(matching_room, move_user);
            match_svr_room_notify_user_other_leave(matching_room, move_user);
            match_svr_room_notify_user_other_join(creating_room, move_user);
            match_svr_room_notify_other_user_join(creating_room, move_user);
        }

        if (matching_room->m_user_count == 0) match_svr_room_destory(matching_room);

        if (match_svr_room_send_create_req(creating_room, room_meta) != 0) {
            CPE_ERROR(svr->m_em, "%s: process_creating_room: send create room to room svr fail!", match_svr_name(svr));
            return;
        }
        creating_room->m_data->creating_req_time = match_svr_cur_time(svr);
    }
}
