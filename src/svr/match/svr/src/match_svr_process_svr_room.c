#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "protocol/svr/room/svr_room_pro.h"
#include "match_svr_ops.h"

int match_svr_room_send_delete_req(match_svr_t svr, uint32_t creating_id, uint64_t room_id) {
    dp_req_t room_pkg = NULL;
    dp_req_t room_pkg_head = NULL;
    SVR_ROOM_REQ_DELETE * delete_req;
    
    room_pkg = match_svr_pkg_buf(svr, sizeof(SVR_ROOM_PKG));
    if (room_pkg == NULL) return -1;

    room_pkg_head = set_pkg_head_find(room_pkg);
    assert(room_pkg_head);

    set_pkg_set_sn(room_pkg_head, creating_id);
    set_pkg_set_category(room_pkg_head, set_pkg_request);
    set_pkg_set_to_svr(room_pkg_head, svr->m_room_svr_type_id, 0);

    ((SVR_ROOM_PKG*)dp_req_data(room_pkg))->cmd = SVR_ROOM_CMD_REQ_DELETE;
    delete_req = &((SVR_ROOM_PKG *)dp_req_data(room_pkg))->data.svr_room_req_delete;

    delete_req->room_id = room_id;

    return match_svr_send_pkg(svr, room_pkg);
}

int match_svr_room_send_create_req(match_svr_room_t room, SVR_MATCH_ROOM_META const * room_meta) {
    match_svr_t svr = room->m_svr;
    dp_req_t room_pkg = NULL;
    dp_req_t room_pkg_head = NULL;
    SVR_ROOM_REQ_CREATE * create_req;
    match_svr_user_t process_user;

    assert(room->m_data->creating_id != 0);

    room_pkg = match_svr_pkg_buf(svr, sizeof(SVR_ROOM_PKG) + sizeof(SVR_ROOM_USER) * room->m_user_count);
    if (room_pkg == NULL) return -1;

    room_pkg_head = set_pkg_head_find(room_pkg);
    assert(room_pkg_head);

    set_pkg_set_sn(room_pkg_head, room->m_data->creating_id);
    set_pkg_set_category(room_pkg_head, set_pkg_request);
    set_pkg_set_to_svr(room_pkg_head, svr->m_room_svr_type_id, 0);

    ((SVR_ROOM_PKG*)dp_req_data(room_pkg))->cmd = SVR_ROOM_CMD_REQ_CREATE;
    create_req = &((SVR_ROOM_PKG *)dp_req_data(room_pkg))->data.svr_room_req_create;

    create_req->room_type = room_meta->runing_room_type;
    create_req->user_count = 0;

    TAILQ_FOREACH(process_user, &room->m_users, m_next) {
        SVR_ROOM_CREATING_USER * room_user = &create_req->users[create_req->user_count++];

        room_user->user_id = process_user->m_data->user_id;
        room_user->user_at_svr_type = process_user->m_data->user_at_svr_type;
        room_user->user_at_svr_id = process_user->m_data->user_at_svr_id;
        room_user->user_data_len = process_user->m_data->user_data_len;
        memcpy(room_user->user_data, process_user->m_data->user_data, process_user->m_data->user_data_len); 
    }

    return match_svr_send_pkg(svr, room_pkg);
}

int match_svr_room_response_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    match_svr_t svr = ctx;
    SVR_ROOM_PKG * pkg;

    pkg = dp_req_data(req);

    switch(pkg->cmd) {
    case SVR_ROOM_CMD_RES_CREATE:
        match_svr_response_room_created(svr, req);
        break;
    case SVR_ROOM_CMD_RES_DELETE:
        break;
    default:
        CPE_ERROR(svr->m_em, "%s: process room response: unknown cmd %d!", match_svr_name(svr), pkg->cmd);
        return -1;
    }

    return 0;
}
