#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "room_svr_ops.h"

void room_svr_room_notify_user_join(room_svr_room_t room, room_svr_user_t user) {
    room_svr_t svr = room->m_svr;
    dp_req_t notify_pkg = NULL;
    dp_req_t notify_pkg_head = NULL;
    SVR_ROOM_NOTIFY_USER_JOIN * notify;
    room_svr_user_t process_user;

    TAILQ_FOREACH(process_user, &room->m_users, m_next) {
        if (process_user == user) continue; /*忽略自己*/
        if (process_user->m_data->user_state != SVR_ROOM_USER_ACTIVE) continue;

        if (notify_pkg == NULL) {
            notify_pkg = room_svr_build_notify(svr, SVR_ROOM_CMD_NOTIFY_USER_JOIN, sizeof(SVR_ROOM_PKG));
            if (notify_pkg == NULL) {
                CPE_ERROR(svr->m_em, "%s: create: notify_other_join, alloc notify pkg fail!", room_svr_name(svr));
                return;
            }

            notify = &((SVR_ROOM_PKG*)dp_req_data(notify_pkg))->data.svr_room_notify_user_join;
            notify->room_id = room->m_data->room_id;
            notify->join_user.user_id = user->m_data->user_id;
            notify->join_user.user_data_len = user->m_data->user_data_len;
            memcpy(notify->join_user.user_data, user->m_data->user_data, user->m_data->user_data_len);

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

        room_svr_send_pkg(svr, notify_pkg);
    }
}

void room_svr_room_notify_user_leave(room_svr_room_t room, room_svr_user_t user, uint32_t reason) {
    room_svr_t svr = room->m_svr;
    dp_req_t notify_pkg = NULL;
    dp_req_t notify_pkg_head = NULL;
    SVR_ROOM_NOTIFY_USER_LEAVE * notify;
    room_svr_user_t process_user;

    TAILQ_FOREACH(process_user, &room->m_users, m_next) {
        if (process_user == user) continue; /*忽略自己*/
        if (process_user->m_data->user_state != SVR_ROOM_USER_ACTIVE) continue;

        if (notify_pkg == NULL) {
            notify_pkg = room_svr_build_notify(svr, SVR_ROOM_CMD_NOTIFY_USER_LEAVE, sizeof(SVR_ROOM_PKG));
            if (notify_pkg == NULL) {
                CPE_ERROR(svr->m_em, "%s: create: notify_other_leave, alloc notify pkg fail!", room_svr_name(svr));
                return;
            }

            notify = &((SVR_ROOM_PKG*)dp_req_data(notify_pkg))->data.svr_room_notify_user_leave;
            notify->room_id = room->m_data->room_id;
            notify->leave_user = user->m_data->user_id;
            notify->leave_reason = reason;

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

        room_svr_send_pkg(svr, notify_pkg);
    }
}

void room_svr_room_notify_room_destoried(room_svr_room_t room, uint8_t reason) {
    room_svr_t svr = room->m_svr;
    dp_req_t notify_pkg = NULL;
    dp_req_t notify_pkg_head = NULL;
    SVR_ROOM_NOTIFY_ROOM_DESTORY * notify;
    room_svr_user_t to_user;

    TAILQ_FOREACH(to_user, &room->m_users, m_next) {
        if (to_user->m_data->user_state != SVR_ROOM_USER_ACTIVE) continue;

        if (notify_pkg == NULL) {
            notify_pkg = room_svr_build_notify(svr, SVR_ROOM_CMD_NOTIFY_ROOM_DESTORY, sizeof(SVR_ROOM_PKG));
            if (notify_pkg == NULL) {
                CPE_ERROR(svr->m_em, "%s: room destoried: notify all user leave, alloc notify pkg fail!", room_svr_name(svr));
                return;
            }
            notify = &((SVR_ROOM_PKG*)dp_req_data(notify_pkg))->data.svr_room_notify_room_destory;
            notify->room_id = room->m_data->room_id;
            notify->destory_reason = reason;

            notify_pkg_head = set_pkg_head_find(notify_pkg);
            assert(notify_pkg_head);
        }

        notify->user_id = to_user->m_data->user_id;

        assert(notify_pkg_head);
        set_pkg_set_to_svr(
            notify_pkg_head,
            to_user->m_data->user_at_svr_type,
            to_user->m_data->user_at_svr_id);

        room_svr_send_pkg(svr, notify_pkg);
    }
}

void room_svr_room_notify_msg(room_svr_room_t room, uint64_t sender, void const * data, uint32_t data_len) {
    room_svr_t svr = room->m_svr;
    dp_req_t notify_pkg = NULL;
    dp_req_t notify_pkg_head = NULL;
    SVR_ROOM_NOTIFY_MSG * notify;
    room_svr_user_t to_user;

    TAILQ_FOREACH(to_user, &room->m_users, m_next) {
        if (to_user->m_data->user_state != SVR_ROOM_USER_ACTIVE) continue;

        if (notify_pkg == NULL) {
            notify_pkg = room_svr_build_notify(svr, SVR_ROOM_CMD_NOTIFY_ROOM_TICK, sizeof(SVR_ROOM_PKG) + data_len);
            if (notify_pkg == NULL) {
                CPE_ERROR(svr->m_em, "%s: room destoried: notify all user leave, alloc notify pkg fail!", room_svr_name(svr));
                return;
            }
            notify = &((SVR_ROOM_PKG*)dp_req_data(notify_pkg))->data.svr_room_notify_msg;
            notify->room_id = room->m_data->room_id;
            notify->send_user = sender;
            notify->data_len = data_len;
            memcpy(notify->data, data, data_len);

            notify_pkg_head = set_pkg_head_find(notify_pkg);
            assert(notify_pkg_head);
        }

        notify->user_id = to_user->m_data->user_id;

        assert(notify_pkg_head);
        set_pkg_set_to_svr(
            notify_pkg_head,
            to_user->m_data->user_at_svr_type,
            to_user->m_data->user_at_svr_id);

        room_svr_send_pkg(svr, notify_pkg);
    }
}

void room_svr_room_notify_tick(room_svr_room_t room) {
    room_svr_t svr = room->m_svr;
    dp_req_t notify_pkg = NULL;
    dp_req_t notify_pkg_head = NULL;
    SVR_ROOM_NOTIFY_ROOM_TICK * notify;
    room_svr_user_t to_user;

    TAILQ_FOREACH(to_user, &room->m_users, m_next) {
        if (to_user->m_data->user_state != SVR_ROOM_USER_ACTIVE) continue;

        if (notify_pkg == NULL) {
            notify_pkg = room_svr_build_notify(svr, SVR_ROOM_CMD_NOTIFY_ROOM_TICK, sizeof(SVR_ROOM_PKG));
            if (notify_pkg == NULL) {
                CPE_ERROR(svr->m_em, "%s: room destoried: notify all user leave, alloc notify pkg fail!", room_svr_name(svr));
                return;
            }
            notify = &((SVR_ROOM_PKG*)dp_req_data(notify_pkg))->data.svr_room_notify_room_tick;
            notify->room_id = room->m_data->room_id;

            notify_pkg_head = set_pkg_head_find(notify_pkg);
            assert(notify_pkg_head);
        }

        notify->user_id = to_user->m_data->user_id;

        assert(notify_pkg_head);
        set_pkg_set_to_svr(
            notify_pkg_head,
            to_user->m_data->user_at_svr_type,
            to_user->m_data->user_at_svr_id);

        room_svr_send_pkg(svr, notify_pkg);
    }
}
