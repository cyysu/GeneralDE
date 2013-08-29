#include <assert.h>
#include "room_svr_ops.h"

void room_svr_timer(void * ctx, gd_timer_id_t timer_id, void * arg) {
    room_svr_t svr;
    uint32_t cur_time;
    int process_room_max;
    int process_room_count;

    svr = ctx;

    cur_time = room_svr_cur_time(svr);

    process_room_max =
        cpe_hash_table_count(&svr->m_rooms) > 100
        ? 100
        : cpe_hash_table_count(&svr->m_rooms);

    for(process_room_count = 0; process_room_count < process_room_max; ++process_room_count) {
        room_svr_room_t room;
        room_svr_user_t user;
        room_svr_user_list_t leave_user_queue; 
        int active_user_count;

        room = TAILQ_FIRST(&svr->m_room_check_queue);
        assert(room);

        TAILQ_REMOVE(&svr->m_room_check_queue, room, m_next_for_check);
        TAILQ_INSERT_TAIL(&svr->m_room_check_queue, room, m_next_for_check);

        TAILQ_INIT(&leave_user_queue);

        active_user_count = 0;
        TAILQ_FOREACH(user, &room->m_users, m_next) {
            if (user->m_data->user_state != SVR_ROOM_USER_ACTIVE) continue;

            if (user->m_data->user_last_op_time + svr->m_timeout_span_s > cur_time) {
                room_svr_user_update_state(user, 0, SVR_ROOM_USER_LEAVE);
                TAILQ_INSERT_TAIL(&leave_user_queue, user, m_next_for_tmp);
            }
            else {
                ++active_user_count;
            }
        }

        if (active_user_count == 0) {
            room_svr_room_destory(room);
        }
        else {
            /*发送所有超时用户的消息*/
            TAILQ_FOREACH(user, &leave_user_queue, m_next_for_tmp) {
                room_svr_room_notify_user_leave(room, user, SVR_ROOM_USER_LEAVE_BY_TIMEOUT);
            }
        }
    }
}

