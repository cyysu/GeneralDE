#ifndef SVR_ROOM_SVR_TYPES_H
#define SVR_ROOM_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/net/net_types.h"
#include "cpe/aom/aom_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "protocol/svr/room/svr_room_internal.h"
#include "protocol/svr/room/svr_room_meta.h"

typedef struct room_svr * room_svr_t;
typedef struct room_svr_room * room_svr_room_t;
typedef struct room_svr_user * room_svr_user_t;

typedef TAILQ_HEAD(room_svr_room_list, room_svr_room) room_svr_room_list_t;
typedef TAILQ_HEAD(room_svr_user_list, room_svr_user) room_svr_user_list_t;

struct room_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    uint32_t m_timeout_span_ms;
    gd_timer_id_t m_check_timer_id;

    uint64_t m_nextRoomId;

    dp_req_t m_outgoing_pkg;
    cpe_hash_string_t m_send_to;
    dp_rsp_t m_recv_at;

    uint16_t m_meta_count;
    SVR_ROOM_ROOM_META * m_metas;

    LPDRMETA m_room_data_meta;
    aom_obj_mgr_t m_room_data_mgr;
    void * m_room_data_buf;

    LPDRMETA m_user_data_meta;
    aom_obj_mgr_t m_user_data_mgr;
    void * m_user_data_buf;

    LPDRMETA m_pkg_meta_req_broadcast;
    LPDRMETA m_pkg_meta_notify_room_created;
    LPDRMETA m_pkg_meta_notify_room_destoried;
    LPDRMETA m_pkg_meta_plugin_room_created;
    LPDRMETA m_pkg_meta_plugin_room_not_exist;

    struct cpe_hash_table m_rooms;
    struct cpe_hash_table m_users;

    room_svr_room_list_t m_room_check_queue;
};

struct room_svr_room {
    room_svr_t m_svr;
    set_svr_svr_info_t m_logic_svr;
    SVR_ROOM_ROOM_RECORD * m_data;
    uint16_t m_user_count;
    room_svr_user_list_t m_users;

    struct cpe_hash_entry m_hh;
    TAILQ_ENTRY(room_svr_room) m_next_for_check;
};

struct room_svr_user {
    room_svr_room_t m_room;
    SVR_ROOM_USER_RECORD * m_data;

    struct cpe_hash_entry m_hh;
    TAILQ_ENTRY(room_svr_user) m_next;
    TAILQ_ENTRY(room_svr_user) m_next_for_tmp;
};

#endif