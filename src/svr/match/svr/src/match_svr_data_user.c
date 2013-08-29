#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "match_svr_ops.h"

match_svr_user_t
match_svr_user_create(match_svr_room_t room, SVR_MATCH_USER_RECORD * record) {
    match_svr_t svr = room->m_svr;
    match_svr_user_t user;

    user = mem_alloc(svr->m_alloc, sizeof(struct match_svr_user));
    if (user == NULL) {
        CPE_ERROR(svr->m_em, "%s: create user: malloc fail!", match_svr_name(svr));
        return NULL;
    }

    user->m_room = room;
    user->m_data = record;

    cpe_hash_entry_init(&user->m_hh);
    if (cpe_hash_table_insert(&svr->m_users, user) != 0) {
        CPE_ERROR(svr->m_em, "%s: create user: insert fail!", match_svr_name(svr));
        mem_free(svr->m_alloc, user);
        return NULL;
    }

    ++user->m_room->m_user_count;
    TAILQ_INSERT_TAIL(&room->m_users, user, m_next);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: user "FMT_UINT64_T": join room(match_room_id="FMT_UINT64_T")!",
            match_svr_name(svr), record->user_id, room->m_data->match_room_id);
    }

    return user;
}

void match_svr_user_free(match_svr_user_t user) {
    match_svr_t svr = user->m_room->m_svr;

    assert(svr);

    assert(user->m_data);
    user->m_data = NULL;

    --user->m_room->m_user_count;
    TAILQ_REMOVE(&user->m_room->m_users, user, m_next);

    cpe_hash_table_remove_by_ins(&svr->m_users, user);

    mem_free(svr->m_alloc, user);
}

void match_svr_user_destory(match_svr_user_t user) {
    match_svr_t svr = user->m_room->m_svr;

    assert(svr);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: user "FMT_UINT64_T": leave room(match_room_id="FMT_UINT64_T")!",
            match_svr_name(svr), user->m_data->user_id, user->m_room->m_data->match_room_id);
    }

    assert(user->m_data);
    aom_obj_free(svr->m_user_data_mgr, user->m_data);
    user->m_data = NULL;

    --user->m_room->m_user_count;
    TAILQ_REMOVE(&user->m_room->m_users, user, m_next);

    cpe_hash_table_remove_by_ins(&svr->m_users, user);

    mem_free(svr->m_alloc, user);
}

void match_svr_user_free_all(match_svr_t svr) {
    struct cpe_hash_it user_it;
    match_svr_user_t user;

    cpe_hash_it_init(&user_it, &svr->m_users);
    user = cpe_hash_it_next(&user_it);
    while(user) {
        match_svr_user_t next = cpe_hash_it_next(&user_it);
        match_svr_user_free(user);
        user = next;
    }
}

void match_svr_user_move_to_room(match_svr_user_t user, match_svr_room_t room) {
    match_svr_t svr = room->m_svr;

    assert(user->m_room);
    assert(user->m_room != room);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: user "FMT_UINT64_T":"
            " move from room(match_room_id="FMT_UINT64_T") "
            " to  room(match_room_id="FMT_UINT64_T")!",
            match_svr_name(svr), user->m_data->user_id, user->m_room->m_data->match_room_id,
            room->m_data->match_room_id);
    }

    --user->m_room->m_user_count;
    TAILQ_REMOVE(&user->m_room->m_users, user, m_next);

    user->m_room = room;
    ++user->m_room->m_user_count;
    TAILQ_INSERT_TAIL(&user->m_room->m_users, user, m_next);

    user->m_data->match_room_id = user->m_room->m_data->match_room_id;
}

match_svr_user_t
match_svr_user_find_in_room(match_svr_t svr, uint64_t user_id, uint64_t match_room_id) {
    SVR_MATCH_USER_RECORD key_data;
    struct match_svr_user key;
    match_svr_user_t user;

    key.m_data = &key_data;
    key_data.user_id = user_id;

    for(user = cpe_hash_table_find(&svr->m_users, &key);
        user != NULL;
        user = cpe_hash_table_find_next(&svr->m_users, user)
        )
    {
        if (user->m_data->match_room_id == match_room_id) return user;
    }

    return NULL;
}

uint32_t match_svr_user_hash(match_svr_user_t user) {
    return (uint32_t)user->m_data->user_id;
}

int match_svr_user_eq(match_svr_user_t l, match_svr_user_t r) {
    return l->m_data->user_id == r->m_data->user_id;
}
