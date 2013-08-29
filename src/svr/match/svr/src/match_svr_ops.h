#ifndef SVR_MATCH_SVR_OPS_H
#define SVR_MATCH_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "match_svr_types.h"
#include "svr/center/agent/center_agent_types.h" 
#include "protocol/svr/match/svr_match_pro.h"

/*operations of match_svr */
match_svr_t
match_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    uint16_t room_svr_type_id,
    mem_allocrator_t alloc,
    error_monitor_t em);

void match_svr_free(match_svr_t svr);

match_svr_t match_svr_find(gd_app_context_t app, cpe_hash_string_t name);
match_svr_t match_svr_find_nc(gd_app_context_t app, const char * name);
const char * match_svr_name(match_svr_t svr);
uint32_t match_svr_cur_time(match_svr_t svr);

int match_svr_set_send_to(match_svr_t svr, const char * send_to);
int match_svr_set_match_require_recv_at(match_svr_t svr, const char * name);
int match_svr_set_check_span(match_svr_t svr, uint32_t span_ms);
int match_svr_room_data_init_from_mem(match_svr_t svr, size_t memory_size);
int match_svr_room_data_init_from_shm(match_svr_t svr, int shm_key);
int match_svr_user_data_init_from_mem(match_svr_t svr, size_t memory_size);
int match_svr_user_data_init_from_shm(match_svr_t svr, int shm_key);

dp_req_t match_svr_pkg_buf(match_svr_t svr, size_t capacity);
dp_req_t match_svr_build_notify(match_svr_t svr, uint32_t cmd, size_t capacity);
dp_req_t match_svr_build_response(match_svr_t svr, dp_req_t req, size_t capacity);

int match_svr_send_pkg(match_svr_t svr, dp_req_t req);

/*room meta operations*/
int match_svr_meta_room_load(match_svr_t svr, cfg_t cfg);
SVR_MATCH_ROOM_META * match_svr_meta_foom_find(match_svr_t svr, uint16_t match_room_type);

/*room operations*/
match_svr_room_t match_svr_room_create(match_svr_t svr, SVR_MATCH_ROOM_RECORD * record);
void match_svr_room_free(match_svr_room_t cli);
void match_svr_room_free_all(match_svr_t svr);
match_svr_room_t match_svr_room_find_matching(match_svr_t svr, uint64_t match_id);
void match_svr_room_destory(match_svr_room_t cli); /*release data and free*/
match_svr_user_t match_svr_room_lsearch_user(match_svr_room_t room, uint64_t user_id);

int match_svr_room_is_full(match_svr_room_t room, SVR_MATCH_ROOM_META const * room_meta);

uint32_t match_svr_room_room_id_hash(match_svr_room_t match);
int match_svr_room_room_id_eq(match_svr_room_t l, match_svr_room_t r);

/*user operations*/
match_svr_user_t match_svr_user_create(match_svr_room_t match, SVR_MATCH_USER_RECORD * record);
void match_svr_user_free(match_svr_user_t usr);
void match_svr_user_destory(match_svr_user_t usr);
void match_svr_user_free_all(match_svr_t svr);

match_svr_user_t match_svr_user_find_in_room(match_svr_t svr, uint64_t user_id, uint64_t match_room_id);

void match_svr_user_move_to_room(match_svr_user_t user, match_svr_room_t room);

uint32_t match_svr_user_hash(match_svr_user_t user);
int match_svr_user_eq(match_svr_user_t l, match_svr_user_t r);

/*to room svr ops*/
int match_svr_room_send_create_req(match_svr_room_t room, SVR_MATCH_ROOM_META const * room_meta);
int match_svr_room_send_delete_req(match_svr_t svr, uint32_t creating_id, uint64_t room_id);

/*match notify operations*/
void match_svr_room_notify_other_user_join(match_svr_room_t room, match_svr_user_t user);
void match_svr_room_notify_other_user_leave(match_svr_room_t room, match_svr_user_t user);

void match_svr_room_notify_user_other_join(match_svr_room_t room, match_svr_user_t user);
void match_svr_room_notify_user_other_leave(match_svr_room_t room, match_svr_user_t user);

void match_svr_room_notify_room_created(match_svr_room_t room, uint16_t room_svr_id, uint64_t room_id);

/*match request ops*/
void match_svr_request_join(match_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);
void match_svr_request_leave(match_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

#endif
