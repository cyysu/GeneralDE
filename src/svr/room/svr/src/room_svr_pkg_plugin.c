#include <assert.h>
#include "cpe/dr/dr_data.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "room_svr_ops.h"

int room_svr_p_notify_room_created(room_svr_t svr, set_svr_svr_info_t logic_svr, room_svr_room_t room) {
    char buf[sizeof(SVR_ROOM_P_NOTIFY_ROOM_CREATED) + sizeof(SVR_ROOM_CREATING_USER) * room->m_user_count];
    SVR_ROOM_P_NOTIFY_ROOM_CREATED * pkg = (SVR_ROOM_P_NOTIFY_ROOM_CREATED*)buf;
    room_svr_user_t user;

    pkg->room_id = room->m_data->room_id;
    pkg->room_type = room->m_data->room_type;
    pkg->user_count = 0;

    TAILQ_FOREACH(user, &room->m_users, m_next) {
        SVR_ROOM_CREATING_USER * notify_user;
        notify_user = &pkg->users[pkg->user_count++];

        notify_user->user_id = user->m_data->user_id;
        notify_user->user_at_svr_type = user->m_data->user_at_svr_type; 
        notify_user->user_at_svr_id = user->m_data->user_at_svr_id; 

        notify_user->user_data_len = user->m_data->user_data_len;
        memcpy(notify_user->user_data, user->m_data->user_data, user->m_data->user_data_len);
    }

    if (set_svr_stub_send_notify_data(
            svr->m_stub, set_svr_svr_info_svr_type_id(logic_svr), 0,
            0,
            pkg, dr_meta_calc_data_len(svr->m_pkg_meta_plugin_room_created, pkg, 0), svr->m_pkg_meta_plugin_room_created,
            NULL, 0)
        != 0)
    {
        CPE_ERROR(svr->m_em, "%s: plugin_room_created to logic svr fail!", room_svr_name(svr));
        return -1;
    }

    return 0;
}

int room_svr_p_notify_room_not_exist(room_svr_t svr, dp_req_t input_pkg, uint64_t room_id) {
    SVR_ROOM_P_NOTIFY_ROOM_NOT_EXIST pkg;
    dp_req_t input_pkg_head;

    pkg.room_id = room_id;

    input_pkg_head = set_pkg_head_find(input_pkg);
    if (input_pkg_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: p_notify_room_not_exist: input pkg have no head!", room_svr_name(svr));
        return -1;
    }

    if (set_svr_stub_send_notify_data(
            svr->m_stub, set_pkg_from_svr_type(input_pkg_head), set_pkg_from_svr_id(input_pkg_head),
            0,
            &pkg, sizeof(pkg), svr->m_pkg_meta_plugin_room_not_exist,
            NULL, 0)
        != 0)
    {
        CPE_ERROR(svr->m_em, "%s: p_notify_room_not_exist: send fail!", room_svr_name(svr));
        return -1;
    }

    return 0;
}

int room_svr_p_notify_room_message(room_svr_t svr, set_svr_svr_info_t logic_svr, SVR_ROOM_REQ_BROADCAST * msg) {
    if (set_svr_stub_send_notify_data(
            svr->m_stub, set_svr_svr_info_svr_type_id(logic_svr), 0,
            0,
            msg, 0, svr->m_pkg_meta_req_broadcast,
            NULL, 0)
        != 0)
    {
        CPE_ERROR(svr->m_em, "%s: plugin_notify_msg to logic svr fail!", room_svr_name(svr));
        return -1;
    }

    return 0;
}
