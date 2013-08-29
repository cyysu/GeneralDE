#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "room_svr_ops.h"

room_svr_user_t
room_svr_user_create(room_svr_room_t room, SVR_ROOM_USER_RECORD * record) {
    room_svr_t svr = room->m_svr;
    room_svr_user_t user;

    user = mem_alloc(svr->m_alloc, sizeof(struct room_svr_user));
    if (user == NULL) {
        CPE_ERROR(svr->m_em, "%s: create user: malloc fail!", room_svr_name(svr));
        return NULL;
    }

    user->m_room = room;
    user->m_data = record;

    cpe_hash_entry_init(&user->m_hh);
    if (cpe_hash_table_insert(&svr->m_users, user) != 0) {
        CPE_ERROR(svr->m_em, "%s: create user: insert fail!", room_svr_name(svr));
        mem_free(svr->m_alloc, user);
        return NULL;
    }

    ++room->m_user_count;
    TAILQ_INSERT_TAIL(&room->m_users, user, m_next);

    return user;
}

void room_svr_user_free(room_svr_user_t user) {
    room_svr_t svr = user->m_room->m_svr;

    assert(svr);

    assert(user->m_data);
    user->m_data = NULL;

    --user->m_room->m_user_count;
    TAILQ_REMOVE(&user->m_room->m_users, user, m_next);

    cpe_hash_table_remove_by_ins(&svr->m_users, user);

    mem_free(svr->m_alloc, user);
}

void room_svr_user_destory(room_svr_user_t user) {
    room_svr_t svr = user->m_room->m_svr;

    assert(svr);

    --user->m_room->m_user_count;
    TAILQ_REMOVE(&user->m_room->m_users, user, m_next);

    cpe_hash_table_remove_by_ins(&svr->m_users, user);

    assert(user->m_data);
    aom_obj_free(svr->m_user_data_mgr, user->m_data);
    user->m_data = NULL;

    mem_free(svr->m_alloc, user);
}

void room_svr_user_update_state(room_svr_user_t user, uint32_t last_op_time, uint8_t new_state) {
    if (last_op_time > 0) {
        assert(last_op_time >= user->m_data->user_last_op_time);
        user->m_data->user_last_op_time = last_op_time;
    }
    user->m_data->user_state = new_state;
}

uint32_t room_svr_user_hash(room_svr_user_t user) {
    return (uint32_t)user->m_data->user_id;
}

int room_svr_user_eq(room_svr_user_t l, room_svr_user_t r) {
    return l->m_data->user_id == r->m_data->user_id;
}
