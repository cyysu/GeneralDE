#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
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

void room_svr_room_notify_room_created_with_users(room_svr_room_t room) {
    room_svr_t svr = room->m_svr;
    dp_req_t notify_pkg = NULL;
    dp_req_t notify_pkg_head = NULL;
    SVR_ROOM_NOTIFY_ROOM_CREATED * notify;
    SVR_ROOM_ROOM_DATA_ROLES * notify_roles;
    room_svr_user_t user;

    assert(room->m_user_count > 0);

    notify_pkg = room_svr_build_notify(
        svr,
        SVR_ROOM_CMD_NOTIFY_ROOM_CREATED,
        sizeof(SVR_ROOM_PKG) + sizeof(SVR_ROOM_CREATING_USER) * room->m_user_count);
    if (notify_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: notify other user leave, alloc notify pkg fail!", room_svr_name(svr));
        return;
    }
    notify = &((SVR_ROOM_PKG*)dp_req_data(notify_pkg))->data.svr_room_notify_room_created;
    notify->room_id = room->m_data->room_id;
    notify->room_type = room->m_data->room_type;
    notify->room_data_type = SVR_ROOM_ROOM_DATA_TYPE_ROLES;

    notify_roles = &notify->room_data.roles;
    notify_roles->user_count = 0;

    notify_pkg_head = set_pkg_head_find(notify_pkg);
    assert(notify_pkg_head);

    TAILQ_FOREACH(user, &room->m_users, m_next) {
        SVR_ROOM_CREATING_USER * notify_user;
        notify_user = &notify_roles->users[notify_roles->user_count++];

        notify_user->user_id = user->m_data->user_id;
        notify_user->user_at_svr_type = user->m_data->user_at_svr_type; 
        notify_user->user_at_svr_id = user->m_data->user_at_svr_id; 

        notify_user->user_data_len = user->m_data->user_data_len;
        memcpy(notify_user->user_data, user->m_data->user_data, user->m_data->user_data_len);
    }

    TAILQ_FOREACH(user, &room->m_users, m_next) {
        notify->user_id = user->m_data->user_id;

        assert(notify_pkg_head);
        set_pkg_set_to_svr(
            notify_pkg_head,
            user->m_data->user_at_svr_type,
            user->m_data->user_at_svr_id);

        room_svr_send_pkg(svr, notify_pkg);
    }
}

void room_svr_room_notify_room_created_with_data(room_svr_room_t room, uint64_t to_user, void const * data, size_t data_len) {
    room_svr_t svr = room->m_svr;

    char buf[sizeof(SVR_ROOM_ROOM_DATA_DATA) + data_len];
    SVR_ROOM_NOTIFY_ROOM_CREATED * notify = (SVR_ROOM_NOTIFY_ROOM_CREATED *)buf;
    SVR_ROOM_ROOM_DATA_DATA * notify_data;

    notify->room_id = room->m_data->room_id;
    notify->room_type = room->m_data->room_type;
    notify->room_data_type = SVR_ROOM_ROOM_DATA_TYPE_DATA;

    notify_data = &notify->room_data.data;
    notify_data->room_data_len = data_len;
    notify_data->room_data_len = data_len;
    memcpy(notify_data->room_data, data, data_len);

    if (to_user) {
        room_svr_user_t user = room_svr_room_lsearch_user(room, to_user);
        if (user == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: notify_room_created_with_data: user "FMT_UINT64_T" not exist in room "FMT_UINT64_T"!",
                room_svr_name(svr), to_user, room->m_data->room_id);
            return;
        }

        notify->user_id = user->m_data->user_id;

        if (set_svr_stub_send_notify_data(
                svr->m_stub, user->m_data->user_at_svr_type, user->m_data->user_at_svr_id, 0,
                notify, 0, svr->m_pkg_meta_notify_room_created, NULL, 0)
            != 0)
        {
            CPE_ERROR(svr->m_em, "%s: notify_room_created_with_data: send pkg fail!", room_svr_name(svr));
            return;
        }
    }
    else {
        room_svr_user_t user;
        TAILQ_FOREACH(user, &room->m_users, m_next) {
            notify->user_id = user->m_data->user_id;

            if (set_svr_stub_send_notify_data(
                    svr->m_stub, user->m_data->user_at_svr_type, user->m_data->user_at_svr_id, 0,
                    notify, 0, svr->m_pkg_meta_notify_room_created, NULL, 0)
                != 0)
            {
                CPE_ERROR(svr->m_em, "%s: notify_room_created_with_data: send pkg fail!", room_svr_name(svr));
                return;
            }
        }
    }
}

