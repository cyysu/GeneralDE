#ifndef SVR_ROOM_SVR_OPS_H
#define SVR_ROOM_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "room_svr_types.h"
#include "svr/center/agent/center_agent_types.h" 
#include "protocol/svr/room/svr_room_pro.h"

/*operations of room_svr */
room_svr_t
room_svr_create(
    gd_app_context_t app,
    const char * name,
    center_agent_t agent,
    mem_allocrator_t alloc,
    error_monitor_t em);

void room_svr_free(room_svr_t svr);

int room_svr_gen_id(room_svr_t svr, uint64_t * room_id);
uint32_t room_svr_cur_time(room_svr_t svr);

room_svr_t room_svr_find(gd_app_context_t app, cpe_hash_string_t name);
room_svr_t room_svr_find_nc(gd_app_context_t app, const char * name);
const char * room_svr_name(room_svr_t svr);

int room_svr_set_send_to(room_svr_t svr, const char * send_to);
int room_svr_set_recv_at(room_svr_t svr, const char * name);
int room_svr_set_check_span(room_svr_t svr, uint32_t span_ms);

dp_req_t room_svr_build_response(room_svr_t svr, dp_req_t req, size_t capacity);
dp_req_t room_svr_build_notify(room_svr_t svr, uint32_t cmd, size_t capacity);
void room_svr_send_pkg(room_svr_t svr, dp_req_t req);

/*room operations*/
room_svr_room_t room_svr_room_create(room_svr_t svr, SVR_ROOM_ROOM_RECORD * record);
void room_svr_room_free(room_svr_room_t cli);
void room_svr_room_free_all(room_svr_t svr);
room_svr_room_t room_svr_room_find(room_svr_t svr, uint64_t room_id);
void room_svr_room_destory(room_svr_room_t cli); /*release data and free*/
room_svr_user_t room_svr_room_lsearch_user(room_svr_room_t room, uint64_t user_id);
int room_svr_room_active_user_count(room_svr_room_t room);
uint32_t room_svr_room_hash(room_svr_room_t room);
int room_svr_room_eq(room_svr_room_t l, room_svr_room_t r);

/*user operations*/
room_svr_user_t room_svr_user_create(room_svr_room_t room, SVR_ROOM_USER_RECORD * record);
void room_svr_user_free(room_svr_user_t usr);
void room_svr_user_destory(room_svr_user_t usr);

void room_svr_user_update_state(room_svr_user_t user, uint32_t last_op_time, uint8_t new_state);

uint32_t room_svr_user_hash(room_svr_user_t user);
int room_svr_user_eq(room_svr_user_t l, room_svr_user_t r);

/*protocol process ops*/
typedef void (*room_svr_op_t)(room_svr_t svr, dp_req_t pkg);
void room_svr_op_create(room_svr_t svr, dp_req_t pkg);
void room_svr_op_delete(room_svr_t svr, dp_req_t pkg);
void room_svr_op_query_by_type(room_svr_t svr, dp_req_t pkg);
void room_svr_op_query_by_user(room_svr_t svr, dp_req_t pkg);
void room_svr_op_join(room_svr_t svr, dp_req_t pkg);
void room_svr_op_leave(room_svr_t svr, dp_req_t pkg);
void room_svr_op_broadcast(room_svr_t svr, dp_req_t pkg);

/*notify pkg ops*/
void room_svr_room_notify_user_join(room_svr_room_t room, room_svr_user_t user);
void room_svr_room_notify_user_leave(room_svr_room_t room, room_svr_user_t user, uint32_t reason);
void room_svr_room_notify_msg(room_svr_room_t room, uint64_t sender, void const * data, uint32_t data_len);
void room_svr_room_notify_tick(room_svr_room_t room);
void room_svr_room_notify_room_destoried(room_svr_room_t room, uint8_t reason);

#endif
