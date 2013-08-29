#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "room_svr_ops.h"

void room_svr_p_op_send_room_data(room_svr_t svr, dp_req_t pkg_body) {
    SVR_ROOM_P_REQ_SEND_ROOM_DATA * pkg;
    room_svr_room_t room;

    pkg = & ((SVR_ROOM_PKG *)dp_req_data(pkg_body))->data.svr_room_p_req_send_room_data;

    room = room_svr_room_find(svr, pkg->room_id);
    if (room == NULL) {
        CPE_ERROR(svr->m_em, "%s: p_op_send_room_data: room "FMT_UINT64_T" not exist!", room_svr_name(svr), pkg->room_id);
        room_svr_p_notify_room_not_exist(svr, pkg_body, pkg->room_id);
        return;
    }

    room_svr_room_notify_room_created_with_data(room, pkg->target_user, pkg->room_data, pkg->room_data_len);
}
