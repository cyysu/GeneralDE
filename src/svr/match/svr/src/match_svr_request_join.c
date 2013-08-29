#include <assert.h> 
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "svr/center/agent/center_agent.h"
#include "svr/set/share/set_pkg.h"
#include "match_svr_ops.h"

static match_svr_user_t match_svr_do_user_join_room(
    match_svr_t svr, match_svr_room_t room, uint32_t cur_time, dp_req_t pkg_head, SVR_MATCH_REQ_JOIN const * req, int16_t * err);
static match_svr_room_t match_svr_do_find_or_create_room(match_svr_t svr, SVR_MATCH_REQ_JOIN const * req, int16_t * err);

void match_svr_request_join(match_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    dp_req_t res_pkg;
    SVR_MATCH_REQ_JOIN * req;
    SVR_MATCH_RES_JOIN * result;
    match_svr_room_t room = NULL;
    match_svr_user_t user = NULL;
    SVR_MATCH_ROOM_META * room_meta;
    int16_t err = SVR_MATCH_ERROR_INTERNAL;
    uint32_t cur_time = match_svr_cur_time(svr);

    req = & ((SVR_MATCH_PKG *)dp_req_data(pkg_body))->data.svr_match_req_join;

    /*获取房间配置数据*/
    room_meta = match_svr_meta_foom_find(svr, req->match_room_type);
    if (room_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: join: match room type %d is unknown!", match_svr_name(svr), req->match_room_type);
        err = SVR_MATCH_ERROR_ROOM_TYPE_UNKNOWN;
        goto REQ_JOIN_MATCH_FAIL;
    }

    /*获取匹配中的房间*/
    room = match_svr_do_find_or_create_room(svr, req, &err);
    if (room == NULL) goto REQ_JOIN_MATCH_FAIL;

    user = match_svr_do_user_join_room(svr, room, cur_time, pkg_head, req, &err);
    if (user == NULL) goto REQ_JOIN_MATCH_FAIL;

    /*发送成功响应*/
    res_pkg = match_svr_build_response(svr, pkg_body, sizeof(SVR_MATCH_PKG) + sizeof(SVR_MATCH_MATCHING_USER) * (room->m_user_count - 1));
    if (res_pkg == NULL) return;
    result = &((SVR_MATCH_PKG*)dp_req_data(res_pkg))->data.svr_match_res_join;
    result->result = 0;
    match_svr_send_pkg(svr, res_pkg);

    if (match_svr_room_is_full(room, room_meta)) {
        /*向svr_room创建房间*/
        if (match_svr_room_send_create_req(room, room_meta) != 0) {
            CPE_ERROR(svr->m_em, "%s: join: send create room to room svr fail!", match_svr_name(svr));
        }
        match_svr_room_destory(room);
    }
    else {
        /*通知所有房间用户加入游戏*/
        if (room_meta->sync_user) {
            match_svr_room_notify_other_user_join(room, user);
            match_svr_room_notify_user_other_join(room, user);
        }
    }

    return;

REQ_JOIN_MATCH_FAIL:
    if (room && room->m_user_count == 0) match_svr_room_destory(room);

    /*发送失败响应*/
    res_pkg = match_svr_build_response(svr, pkg_body, sizeof(SVR_MATCH_PKG));
    if (res_pkg == NULL) return;
    result = &((SVR_MATCH_PKG*)dp_req_data(res_pkg))->data.svr_match_res_join;
    result->result = err;
    match_svr_send_pkg(svr, res_pkg);
}

static match_svr_user_t
match_svr_do_user_join_room(
    match_svr_t svr, match_svr_room_t room, uint32_t cur_time, 
    dp_req_t pkg_head, SVR_MATCH_REQ_JOIN const * req, int16_t * err) 
{
    match_svr_user_t user;

    user = match_svr_room_lsearch_user(room, req->user.user_id);
    if (user == NULL) {
        SVR_MATCH_USER_RECORD * user_record = NULL;

        user_record = aom_obj_alloc(svr->m_user_data_mgr);
        if (user_record == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: alloc SVR_MATCH_USER_RECORD fail!", match_svr_name(svr));
            return NULL;
        }

        user_record->match_room_id = req->match_room_id;
        user_record->user_id = req->user.user_id;
        user_record->user_join_time = cur_time;

        user = match_svr_user_create(room, user_record);
        if (user == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: generate room id fail!", match_svr_name(svr));
            aom_obj_free(svr->m_user_data_mgr, user_record);
            return NULL;
        }
    }
    else {
        assert(user->m_data->match_room_id == req->match_room_id);
        assert(user->m_data->user_id == req->user.user_id);
    }

    assert(user);
    user->m_data->user_last_op_time = cur_time;
    user->m_data->user_at_svr_type = set_pkg_from_svr_type(pkg_head);
    user->m_data->user_at_svr_id = set_pkg_from_svr_id(pkg_head);
    user->m_data->user_data_len = req->user.user_data_len;
    memcpy(user->m_data->user_data, req->user.user_data, req->user.user_data_len);
        
    return user;
}

static match_svr_room_t
match_svr_do_find_or_create_room(match_svr_t svr, SVR_MATCH_REQ_JOIN const * req, int16_t * err) {
    match_svr_room_t room = NULL;

    room = match_svr_room_find_matching(svr, req->match_room_id);
    if (room) {
        if (room->m_data->match_room_type != req->match_room_id) {
            CPE_ERROR(
                svr->m_em, "%s: join: room type %d <==> %d mismatch!",
                match_svr_name(svr), room->m_data->match_room_type, req->match_room_type);
            *err = SVR_MATCH_ERROR_ROOM_TYPE_MISMATCH;
            return NULL;
        }
    }
    else {
        SVR_MATCH_ROOM_RECORD * room_record = NULL;

        room_record = aom_obj_alloc(svr->m_room_data_mgr);
        if (room_record == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: alloc SVR_MATCH_ROOM_RECORD fail!", match_svr_name(svr));
            return NULL;
        }

        room_record->match_room_id = req->match_room_id;
        room_record->match_room_type = req->match_room_type;

        room = match_svr_room_create(svr, room_record);
        if (room == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: create room fail!", match_svr_name(svr));
            aom_obj_free(svr->m_room_data_mgr, room_record);
            return NULL;
        }
    }

    return room;
}

