#ifndef SVR_MATCH_SVR_TYPES_H
#define SVR_MATCH_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/net/net_types.h"
#include "cpe/aom/aom_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "protocol/svr/match/svr_match_internal.h"
#include "protocol/svr/match/svr_match_meta.h"

typedef struct match_svr * match_svr_t;
typedef struct match_svr_room * match_svr_room_t;
typedef struct match_svr_user * match_svr_user_t;

typedef TAILQ_HEAD(match_svr_user_list, match_svr_user) match_svr_user_list_t;

struct match_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
    uint32_t m_creating_max_id;
    uint16_t m_room_svr_type_id;

    uint32_t m_create_retry_span_s;
    gd_timer_id_t m_check_timer_id;

    dp_rsp_t m_match_require_recv_at;
    dp_rsp_t m_room_response_recv_at;

    dp_req_t m_outgoing_pkg;
    cpe_hash_string_t m_send_to;
    aom_obj_mgr_t m_room_data_mgr;
    aom_obj_mgr_t m_user_data_mgr;

    uint16_t m_meta_count;
    SVR_MATCH_ROOM_META * m_metas;

    struct cpe_hash_table m_users;
    struct cpe_hash_table m_matching_rooms;
    struct cpe_hash_table m_creating_rooms;
};

/* int match_svr_start_ */
/* int gd_timer_mgr_regist_timer( */
/*     gd_timer_mgr_t mgr, */
/*     gd_timer_id_t * id, */
/*     gd_timer_process_fun_t fun, void * ctx, */
/*     void * arg, void (*arg_fini)(void *), */
/*     tl_time_span_t delay, tl_time_span_t span, int repeatCount); */

struct match_svr_room {
    match_svr_t m_svr;
    SVR_MATCH_ROOM_RECORD * m_data;
    uint16_t m_user_count;
    match_svr_user_list_t m_users;

    struct cpe_hash_entry m_hh;
};

struct match_svr_user {
    match_svr_room_t m_room;
    SVR_MATCH_USER_RECORD * m_data;
    struct cpe_hash_entry m_hh;
    TAILQ_ENTRY(match_svr_user) m_next;
};

typedef void (*match_svr_op_t)(match_svr_t svr, dp_req_t pkg);

#endif
