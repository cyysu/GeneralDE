#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "room_svr_ops.h"

room_svr_room_t
room_svr_room_create(room_svr_t svr, SVR_ROOM_ROOM_RECORD * record) {
    room_svr_room_t room;
    room = mem_alloc(svr->m_alloc, sizeof(struct room_svr_room));
    if (room == NULL) {
        CPE_ERROR(svr->m_em, "%s: create room: malloc fail!", room_svr_name(svr));
        return NULL;
    }

    room->m_svr = svr;
    room->m_logic_svr = NULL;
    room->m_data = record;
    room->m_user_count = 0;
    TAILQ_INIT(&room->m_users);

    cpe_hash_entry_init(&room->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_rooms, room) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: create room: insert fail, "FMT_UINT64_T" already exist!",
            room_svr_name(svr), record->room_id);
        mem_free(svr->m_alloc, room);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&svr->m_room_check_queue, room, m_next_for_check);

    return room;
}

void room_svr_room_free(room_svr_room_t room) {
    room_svr_t svr = room->m_svr;
    assert(svr);

    TAILQ_REMOVE(&svr->m_room_check_queue, room, m_next_for_check);

    while(!TAILQ_EMPTY(&room->m_users)) {
        room_svr_user_free(TAILQ_FIRST(&room->m_users));
    }

    /*clear data*/
    assert(room->m_data);
    room->m_data = NULL;

    /*remove from svr*/
    cpe_hash_table_remove_by_ins(&svr->m_rooms, room);

    mem_free(svr->m_alloc, room);
}

void room_svr_room_free_all(room_svr_t svr) {
    struct cpe_hash_it room_it;
    room_svr_room_t room;

    cpe_hash_it_init(&room_it, &svr->m_rooms);

    room = cpe_hash_it_next(&room_it);
    while(room) {
        room_svr_room_t next = cpe_hash_it_next(&room_it);
        room_svr_room_free(room);
        room = next;
    }
}

void room_svr_room_destory(room_svr_room_t room) {
    room_svr_t svr = room->m_svr;

    TAILQ_REMOVE(&svr->m_room_check_queue, room, m_next_for_check);

    while(!TAILQ_EMPTY(&room->m_users)) {
        room_svr_user_destory(TAILQ_FIRST(&room->m_users));
    }

    aom_obj_free(svr->m_room_data_mgr, room->m_data);
    room->m_data = NULL;
    
    /*remove from svr*/
    cpe_hash_table_remove_by_ins(&svr->m_rooms, room);

    mem_free(svr->m_alloc, room);
}

room_svr_room_t
room_svr_room_find(room_svr_t svr, uint64_t room_id) {
    SVR_ROOM_ROOM_RECORD key_data;
    struct room_svr_room key;

    key.m_data = &key_data;
    key_data.room_id = room_id;

    return cpe_hash_table_find(&svr->m_rooms, &key);
}

room_svr_user_t
room_svr_room_lsearch_user(room_svr_room_t room, uint64_t user_id) {
    room_svr_user_t user;

    TAILQ_FOREACH(user, &room->m_users, m_next) {
        if (user->m_data->user_id == user_id) return user;
    }

    return NULL;
}

int room_svr_room_active_user_count(room_svr_room_t room) {
    int r = 0;
    room_svr_user_t user;

    TAILQ_FOREACH(user, &room->m_users, m_next) {
        if (user->m_data->user_state == SVR_ROOM_USER_ACTIVE) {
            ++r;
        }
    }

    return r;
}

uint32_t room_svr_room_hash(room_svr_room_t room) {
    return (uint32_t)room->m_data->room_id;
}

int room_svr_room_eq(room_svr_room_t l, room_svr_room_t r) {
    return l->m_data->room_id == r->m_data->room_id;
}
