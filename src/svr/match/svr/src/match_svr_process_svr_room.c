#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "protocol/svr/room/svr_room_pro.h"
#include "match_svr_ops.h"

int match_svr_room_send_delete_req(match_svr_t svr, uint32_t creating_id, uint64_t room_id) {
    SVR_ROOM_REQ_DELETE delete_req;
    
    delete_req.room_id = room_id;

    if (set_svr_stub_send_req_data(
            svr->m_stub, svr->m_room_svr_type_id, 0, 0,
            &delete_req, sizeof(delete_req), svr->m_room_meta_req_delete_room,
            NULL, 0) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: send svr_room_req_delete fail!", match_svr_name(svr));
        return -1;
    }

    return 0;
}

int match_svr_room_send_create_req(match_svr_room_t room, SVR_MATCH_ROOM_META const * room_meta) {
    match_svr_t svr = room->m_svr;
    char buf[sizeof(SVR_ROOM_REQ_CREATE) + sizeof(SVR_ROOM_USER) * room->m_user_count];
    SVR_ROOM_REQ_CREATE * create_req = (SVR_ROOM_REQ_CREATE *)buf;
    match_svr_user_t process_user;

    create_req->room_type = room_meta->runing_room_type;
    create_req->user_count = 0;

    TAILQ_FOREACH(process_user, &room->m_users, m_next) {
        SVR_ROOM_CREATING_USER * room_user = &create_req->users[create_req->user_count++];

        assert(process_user->m_data->user_data_len <= SVR_ROOM_USER_DATA_MAX);

        room_user->user_id = process_user->m_data->user_id;
        room_user->user_at_svr_type = process_user->m_data->user_at_svr_type;
        room_user->user_at_svr_id = process_user->m_data->user_at_svr_id;
        room_user->user_data_len = process_user->m_data->user_data_len;
        memcpy(room_user->user_data, process_user->m_data->user_data, process_user->m_data->user_data_len); 
    }

    if (set_svr_stub_send_req_data(
            svr->m_stub, svr->m_room_svr_type_id, 0, 0,
            create_req, 0, svr->m_room_meta_req_create_room,
            NULL, 0) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: send svr_room_req_create fail!", match_svr_name(svr));
        return -1;
    }

    return 0;
}
