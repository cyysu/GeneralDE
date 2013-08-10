#include <assert.h>
#include "match_svr_ops.h"

void match_svr_timer(void * ctx, gd_timer_id_t timer_id, void * arg) {
    match_svr_t svr = ctx;
    struct cpe_hash_it room_it;
    match_svr_room_t room;
    match_svr_room_t next;
    SVR_MATCH_ROOM_META * room_meta;
    uint32_t cur_time = match_svr_cur_time(svr);

    cpe_hash_it_init(&room_it, &svr->m_creating_rooms);
    for(room = cpe_hash_it_next(&room_it); room; room = next) {
        next = cpe_hash_it_next(&room_it);

        if (room->m_data->creating_req_time == 0) {
            room_meta = match_svr_meta_foom_find(svr, room->m_data->match_room_type);
            assert(room_meta);
            if (match_svr_room_send_create_req(room, room_meta)) {
                room->m_data->creating_req_time = cur_time;
            }
        }
        else if (room->m_data->creating_req_time + svr->m_create_retry_span_s < cur_time) { /*创建房间超时*/
            match_svr_room_to_creating(room);

            room_meta = match_svr_meta_foom_find(svr, room->m_data->match_room_type);
            assert(room_meta);
            if (match_svr_room_send_create_req(room, room_meta)) {
                room->m_data->creating_req_time = cur_time;
            }
        }
    }
}

