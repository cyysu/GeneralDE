#include <assert.h> 
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "room_svr_ops.h"

static int room_svr_op_create_user(room_svr_t svr, room_svr_room_t room, uint32_t cur_time, SVR_ROOM_CREATING_USER const * req_user) {
    room_svr_user_t user;
    SVR_ROOM_USER_RECORD * user_record = NULL;

    user_record = aom_obj_alloc(svr->m_user_data_mgr);
    if (user_record == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: alloc SVR_ROOM_USER_RECORD fail!", room_svr_name(svr));
        return -1;
    }

    user_record->room_id = room->m_data->room_id;
    user_record->user_id = req_user->user_id;
    user_record->user_at_svr_type = req_user->user_at_svr_type;
    user_record->user_at_svr_id = req_user->user_at_svr_id;
    user_record->user_state = SVR_ROOM_USER_ACTIVE;
    user_record->user_join_time = cur_time;
    user_record->user_last_op_time = cur_time;
    user_record->user_data_len = req_user->user_data_len;
    memcpy(user_record->user_data, req_user->user_data, req_user->user_data_len);

    user = room_svr_user_create(room, user_record);
    if (user == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: generate room id fail!", room_svr_name(svr));
        aom_obj_free(svr->m_user_data_mgr, user_record);
        return -1;
    }

    return 0;
}

static room_svr_room_t room_svr_op_create_room(room_svr_t svr, SVR_ROOM_REQ_CREATE const * req) {
    SVR_ROOM_ROOM_META * room_meta;
    SVR_ROOM_ROOM_RECORD * room_record = NULL;
    set_svr_svr_info_t logic_svr = NULL;
    room_svr_room_t room = NULL;
    uint16_t user_pos = 0;
    uint32_t cur_time = room_svr_cur_time(svr);

    room_meta = room_svr_meta_foom_find(svr, req->room_type);
    if (room_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: room meta of type %d not exist!", room_svr_name(svr), req->room_type);
        return NULL;
    }

    if (room_meta->logic_svr[0] != 0) {
        logic_svr = set_svr_svr_info_find_by_name(svr->m_stub, room_meta->logic_svr);
        if (logic_svr == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: logic svr %s not exist!", room_svr_name(svr), room_meta->logic_svr);
            return NULL;
        }
    }

    room_record = aom_obj_alloc(svr->m_room_data_mgr);
    if (room_record == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: alloc SVR_ROOM_ROOM_RECORD fail!", room_svr_name(svr));
        return NULL;
    }

    if (room_svr_gen_id(svr, &room_record->room_id) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: generate room id fail!", room_svr_name(svr));
        aom_obj_free(svr->m_room_data_mgr, room_record);
        return NULL;
    }
    room_record->room_type = req->room_type;
    room_record->creation_time = cur_time;

    room = room_svr_room_create(svr, room_record);
    if (room == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: create room fail!", room_svr_name(svr));
        aom_obj_free(svr->m_room_data_mgr, room_record);
        return NULL;
    }

    for(user_pos = 0; user_pos < req->user_count; ++user_pos) {
        if (room_svr_op_create_user(svr, room, cur_time, &req->users[user_pos]) != 0) {
            room_svr_room_destory(room);
            return NULL;
        }
    }

    if (logic_svr) {
        if (room_svr_p_notify_room_created(svr, logic_svr, room) != 0) {
            room_svr_room_destory(room);
            return NULL;
        }

        room->m_logic_svr = logic_svr;
    }
    else {
        room_svr_room_notify_room_created_with_users(room);
    }

    return room;
}

void room_svr_op_create(room_svr_t svr, dp_req_t agent_pkg) {
    dp_req_t res_pkg;
    SVR_ROOM_REQ_CREATE * req;
    SVR_ROOM_RES_CREATE * result;
    room_svr_room_t room = NULL;

    req = & ((SVR_ROOM_PKG *)dp_req_data(agent_pkg))->data.svr_room_req_create;

    room = room_svr_op_create_room(svr, req);
    if (room == NULL) goto REQ_CREATE_ROOM_FAIL;

    /*发送成功响应*/
    res_pkg = room_svr_build_response(svr, agent_pkg, sizeof(SVR_ROOM_PKG));
    if (res_pkg == NULL) return;
    result = &((SVR_ROOM_PKG *)dp_req_data(res_pkg))->data.svr_room_res_create;
    result->result = 0;
    result->room_id = room->m_data->room_id;
    room_svr_send_pkg(svr, res_pkg);

    return;

REQ_CREATE_ROOM_FAIL:
    if (room) room_svr_room_destory(room);

    /*发送失败响应*/
    res_pkg = room_svr_build_response(svr, agent_pkg, sizeof(SVR_ROOM_PKG));
    if (res_pkg == NULL) return;
    result = &((SVR_ROOM_PKG *)dp_req_data(res_pkg))->data.svr_room_res_create;
    result->result = -1;
    result->room_id = 0;
    room_svr_send_pkg(svr, res_pkg);
}
