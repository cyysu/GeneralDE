#include <assert.h>
#include "room_svr_ops.h"

void room_svr_timer(void * ctx, gd_timer_id_t timer_id, void * arg) {
    room_svr_t svr = ctx;
    room_svr_user_t user;
    room_svr_room_t room;
    uint32_t cur_time = room_svr_cur_time(svr);

    /*检查超时的用户*/
    while((user = TAILQ_FIRST(&svr->m_user_check_queue))) {
        room_svr_user_list_t leave_user_queue;
        int active_user_count;

        assert(user->m_data->user_state == SVR_ROOM_USER_ACTIVE);

        if (user->m_data->user_last_op_time + svr->m_timeout_span_s > cur_time) break;

        /*以room为单位进行处理，避免向已经离线的用户发送消息*/
        room = user->m_room;

        TAILQ_INIT(&leave_user_queue);

        /*首先检查room中所有的用户是否超时*/
        active_user_count = 0;
        TAILQ_FOREACH(user, &room->m_users, m_next) {
            if (user->m_data->user_state != SVR_ROOM_USER_ACTIVE) continue;

            if (user->m_data->user_last_op_time + svr->m_timeout_span_s > cur_time) {
                room_svr_user_update_state(user, 0, SVR_ROOM_USER_LEAVE);
                TAILQ_INSERT_TAIL(&leave_user_queue, user, m_next_for_check);
            }
            else {
                ++active_user_count;
            }
        }

        if (active_user_count > 0) {
            /*发送所有超时用户的消息*/
            TAILQ_FOREACH(user, &leave_user_queue, m_next) {
                room_svr_room_notify_user_leave(room, user, SVR_ROOM_USER_LEAVE_BY_TIMEOUT);
            }
        }
        else {
            room_svr_room_destory(room);
        }
    }
}

