#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "match_svr_ops.h"

match_svr_room_t
match_svr_room_create(match_svr_t svr, SVR_MATCH_ROOM_RECORD * record) {
    match_svr_room_t room;
    room = mem_alloc(svr->m_alloc, sizeof(struct match_svr_room));
    if (room == NULL) {
        CPE_ERROR(svr->m_em, "%s: create room: malloc fail!", match_svr_name(svr));
        return NULL;
    }

    room->m_svr = svr;
    room->m_data = record;
    room->m_user_count = 0;

    TAILQ_INIT(&room->m_users);

    cpe_hash_entry_init(&room->m_hh);

    if (record->creating_id != 0) {
        if (cpe_hash_table_insert_unique(&svr->m_creating_rooms, room) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: create room: insert fail, creating id %d already exist!",
                match_svr_name(svr), record->creating_id);
            mem_free(svr->m_alloc, room);
            return NULL;
        }
    }
    else {
        if (cpe_hash_table_insert_unique(&svr->m_matching_rooms, room) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: create room: insert fail, room id "FMT_UINT64_T" already exist!",
                match_svr_name(svr), record->match_room_id);
            mem_free(svr->m_alloc, room);
            return NULL;
        }
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: room(match_room_id="FMT_UINT64_T", creating_id="FMT_UINT32_T"): created!",
            match_svr_name(svr), record->match_room_id, record->creating_id);
    }

    return room;
}

void match_svr_room_free(match_svr_room_t room) {
    match_svr_t svr = room->m_svr;
    assert(svr);

    while(!TAILQ_EMPTY(&room->m_users)) {
        match_svr_user_free(TAILQ_FIRST(&room->m_users));
    }

    assert(room->m_user_count == 0);

    /*clear data*/
    assert(room->m_data);
    room->m_data = NULL;

    /*remove from svr*/
    if (room->m_data->creating_id == 0) {
        cpe_hash_table_remove_by_ins(&svr->m_matching_rooms, room);
    }
    else {
        cpe_hash_table_remove_by_ins(&svr->m_creating_rooms, room);
    }

    mem_free(svr->m_alloc, room);
}

void match_svr_room_free_all(match_svr_t svr) {
    struct cpe_hash_it room_it;
    match_svr_room_t room;

    cpe_hash_it_init(&room_it, &svr->m_matching_rooms);
    room = cpe_hash_it_next(&room_it);
    while(room) {
        match_svr_room_t next = cpe_hash_it_next(&room_it);
        match_svr_room_free(room);
        room = next;
    }

    cpe_hash_it_init(&room_it, &svr->m_creating_rooms);
    room = cpe_hash_it_next(&room_it);
    while(room) {
        match_svr_room_t next = cpe_hash_it_next(&room_it);
        match_svr_room_free(room);
        room = next;
    }
}

void match_svr_room_destory(match_svr_room_t room) {
    match_svr_t svr = room->m_svr;

    while(!TAILQ_EMPTY(&room->m_users)) {
        match_svr_user_destory(TAILQ_FIRST(&room->m_users));
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: room(match_room_id="FMT_UINT64_T", creating_id="FMT_UINT32_T"): destoried!",
            match_svr_name(svr), room->m_data->match_room_id, room->m_data->creating_id);
    }

    aom_obj_free(svr->m_room_data_mgr, room->m_data);
    room->m_data = NULL;
    
    /*remove from svr*/
    if (room->m_data->creating_id == 0) {
        cpe_hash_table_remove_by_ins(&svr->m_matching_rooms, room);
    }
    else {
        cpe_hash_table_remove_by_ins(&svr->m_creating_rooms, room);
    }

    mem_free(svr->m_alloc, room);
}

int match_svr_room_is_full(match_svr_room_t room, SVR_MATCH_ROOM_META const * room_meta) {
    return room->m_user_count >= room_meta->require_user_count
        ? 1
        : 0;
}

void match_svr_room_to_creating(match_svr_room_t room) {
    match_svr_t svr = room->m_svr;
    match_svr_user_t user;
    int rv;
    uint32_t new_creating_id = ++svr->m_creating_max_id;

    if (room->m_data->creating_id == 0) {
        cpe_hash_table_remove_by_ins(&svr->m_matching_rooms, room);
    }
    else {
        cpe_hash_table_remove_by_ins(&svr->m_creating_rooms, room);
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: room(match_room_id="FMT_UINT64_T", creating_id="FMT_UINT32_T"): changed to creating(creating_id="FMT_UINT32_T")!",
            match_svr_name(svr), room->m_data->match_room_id, room->m_data->creating_id, new_creating_id);
    }

    room->m_data->creating_id = new_creating_id;
    room->m_data->creating_req_time = 0;

    cpe_hash_entry_init(&room->m_hh);
    rv = cpe_hash_table_insert_unique(&svr->m_creating_rooms, room);
    assert(rv == 0);

    TAILQ_FOREACH(user, &room->m_users, m_next) {
        user->m_data->creating_id = room->m_data->creating_id;
    }
}

void match_svr_room_to_matching(match_svr_room_t room) {
    match_svr_t svr = room->m_svr;
    match_svr_user_t user;
    int rv;

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: room(match_room_id="FMT_UINT64_T", creating_id="FMT_UINT32_T"): changed to matching!",
            match_svr_name(svr), room->m_data->match_room_id, room->m_data->creating_id);
    }

    assert(room->m_data->creating_id != 0);

    cpe_hash_table_remove_by_ins(&svr->m_creating_rooms, room);

    room->m_data->creating_id = 0;
    room->m_data->creating_req_time = 0;

    cpe_hash_entry_init(&room->m_hh);
    rv = cpe_hash_table_insert_unique(&svr->m_matching_rooms, room);
    assert(rv == 0);

    TAILQ_FOREACH(user, &room->m_users, m_next) {
        user->m_data->creating_id = room->m_data->creating_id;
    }
}

match_svr_room_t
match_svr_room_find_matching(match_svr_t svr, uint64_t room_id) {
    SVR_MATCH_ROOM_RECORD key_data;
    struct match_svr_room key;

    key.m_data = &key_data;
    key_data.match_room_id = room_id;

    return cpe_hash_table_find(&svr->m_matching_rooms, &key);
}

match_svr_room_t
match_svr_room_find_creating(match_svr_t svr, uint32_t creating_id) {
    SVR_MATCH_ROOM_RECORD key_data;
    struct match_svr_room key;

    key.m_data = &key_data;
    key_data.creating_id = creating_id;

    return cpe_hash_table_find(&svr->m_creating_rooms, &key);
}

match_svr_user_t
match_svr_room_lsearch_user(match_svr_room_t room, uint64_t user_id) {
    match_svr_user_t user;

    TAILQ_FOREACH(user, &room->m_users, m_next) {
        if (user->m_data->user_id == user_id) return user;
    }

    return NULL;
}

uint32_t match_svr_room_room_id_hash(match_svr_room_t room) {
    return (uint32_t)room->m_data->match_room_id;
}

int match_svr_room_room_id_eq(match_svr_room_t l, match_svr_room_t r) {
    return l->m_data->match_room_id == r->m_data->match_room_id;
}

uint32_t match_svr_room_creating_id_hash(match_svr_room_t room) {
    return room->m_data->creating_id;
}

int match_svr_room_creating_id_eq(match_svr_room_t l, match_svr_room_t r) {
    return l->m_data->creating_id == r->m_data->creating_id;
}
