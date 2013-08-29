#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "match_svr_ops.h"

void match_svr_room_notify_other_user_join(match_svr_room_t room, match_svr_user_t user) {
    match_svr_t svr = room->m_svr;
    dp_req_t notify_pkg = NULL;
    dp_req_t notify_pkg_head = NULL;
    SVR_MATCH_NOTIFY_JOIN * notify;
    match_svr_user_t process_user;

    TAILQ_FOREACH(process_user, &room->m_users, m_next) {
        if (user == process_user) continue; /*忽略自己*/

        if (notify_pkg == NULL) {
            notify_pkg = match_svr_build_notify(svr, SVR_MATCH_CMD_NOTIFY_JOIN, sizeof(SVR_MATCH_PKG));
            if (notify_pkg == NULL) {
                CPE_ERROR(svr->m_em, "%s: create: notify_other_join, alloc notify pkg fail!", match_svr_name(svr));
                return;
            }

            notify_pkg_head = set_pkg_head_find(notify_pkg);
            assert(notify_pkg_head);

            notify = &((SVR_MATCH_PKG*)dp_req_data(notify_pkg))->data.svr_match_notify_join;
            notify->match_room_id = room->m_data->match_room_id;
            notify->join_user.user_id = user->m_data->user_id;
            notify->join_user.user_data_len = user->m_data->user_data_len;
            memcpy(notify->join_user.user_data, user->m_data->user_data, user->m_data->user_data_len);
        }

        assert(notify);
        notify->user_id = process_user->m_data->user_id;

        assert(notify_pkg_head);
        set_pkg_set_to_svr(
            notify_pkg_head,
            process_user->m_data->user_at_svr_type,
            process_user->m_data->user_at_svr_id);

        match_svr_send_pkg(svr, notify_pkg);
    }
}

void match_svr_room_notify_user_other_join(match_svr_room_t room, match_svr_user_t user) {
    match_svr_t svr = room->m_svr;
    dp_req_t notify_pkg = NULL;
    SVR_MATCH_NOTIFY_JOIN * notify;
    match_svr_user_t process_user;

    TAILQ_FOREACH(process_user, &room->m_users, m_next) {
        if (user == process_user) continue; /*忽略自己*/

        if (notify_pkg == NULL) {
            notify_pkg = match_svr_build_notify(svr, SVR_MATCH_CMD_NOTIFY_JOIN, sizeof(SVR_MATCH_PKG));
            if (notify_pkg == NULL) {
                CPE_ERROR(svr->m_em, "%s: create: notify_other_join, alloc notify pkg fail!", match_svr_name(svr));
                return;
            }

            notify = &((SVR_MATCH_PKG*)dp_req_data(notify_pkg))->data.svr_match_notify_join;
            notify->match_room_id = room->m_data->match_room_id;
            notify->user_id = user->m_data->user_id;
        }

        assert(notify);

        notify->join_user.user_id = process_user->m_data->user_id;
        notify->join_user.user_data_len = process_user->m_data->user_data_len;
        memcpy(notify->join_user.user_data, process_user->m_data->user_data, process_user->m_data->user_data_len);

        set_pkg_set_to_svr(
            notify_pkg,
            process_user->m_data->user_at_svr_type,
            process_user->m_data->user_at_svr_id);

        match_svr_send_pkg(svr, notify_pkg);
    }
}

void match_svr_room_notify_other_user_leave(match_svr_room_t room, match_svr_user_t user) {
    match_svr_t svr = room->m_svr;
    dp_req_t notify_pkg = NULL;
    dp_req_t notify_pkg_head;
    SVR_MATCH_NOTIFY_LEAVE * notify;
    match_svr_user_t process_user;

    TAILQ_FOREACH(process_user, &room->m_users, m_next) {
        if (user == process_user) continue; /*忽略自己*/

        if (notify_pkg == NULL) {
            notify_pkg = match_svr_build_notify(svr, SVR_MATCH_CMD_NOTIFY_LEAVE, sizeof(SVR_MATCH_PKG));
            if (notify_pkg == NULL) {
                CPE_ERROR(svr->m_em, "%s: create: notify other user leave, alloc notify pkg fail!", match_svr_name(svr));
                return;
            }

            notify = &((SVR_MATCH_PKG*)dp_req_data(notify_pkg))->data.svr_match_notify_leave;
            notify->match_room_id = room->m_data->match_room_id;
            notify->leave_user_id = user->m_data->user_id;

            notify_pkg_head = set_pkg_head_find(notify_pkg);
            assert(notify_pkg_head);
        }

        assert(notify);
        notify->user_id = process_user->m_data->user_id;

        assert(notify_pkg_head);
        set_pkg_set_to_svr(
            notify_pkg_head,
            process_user->m_data->user_at_svr_type,
            process_user->m_data->user_at_svr_id);

        match_svr_send_pkg(svr, notify_pkg);
    }
}

void match_svr_room_notify_user_other_leave(match_svr_room_t room, match_svr_user_t user) {
    match_svr_t svr = room->m_svr;
    dp_req_t notify_pkg = NULL;
    dp_req_t notify_pkg_head = NULL;
    SVR_MATCH_NOTIFY_LEAVE * notify;
    match_svr_user_t process_user;

    TAILQ_FOREACH(process_user, &room->m_users, m_next) {
        if (user == process_user) continue; /*忽略自己*/

        if (notify_pkg == NULL) {
            notify_pkg = match_svr_build_notify(svr, SVR_MATCH_CMD_NOTIFY_LEAVE, sizeof(SVR_MATCH_PKG));
            if (notify_pkg == NULL) {
                CPE_ERROR(svr->m_em, "%s: create: notify other user leave, alloc notify pkg fail!", match_svr_name(svr));
                return;
            }

            notify = &((SVR_MATCH_PKG*)dp_req_data(notify_pkg))->data.svr_match_notify_leave;
            notify->match_room_id = room->m_data->match_room_id;
            notify->user_id = user->m_data->user_id;

            notify_pkg_head = set_pkg_head_find(notify_pkg);
            assert(notify_pkg_head);
        }

        assert(notify);
        notify->leave_user_id = process_user->m_data->user_id;

        assert(notify_pkg_head);
        set_pkg_set_to_svr(
            notify_pkg_head,
            process_user->m_data->user_at_svr_type,
            process_user->m_data->user_at_svr_id);

        match_svr_send_pkg(svr, notify_pkg);
    }
}
