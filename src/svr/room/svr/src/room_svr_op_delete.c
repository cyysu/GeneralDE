#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "room_svr_ops.h"

void room_svr_op_delete(room_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body) {
    dp_req_t res_pkg;
    SVR_ROOM_REQ_DELETE * req;
    SVR_ROOM_RES_DELETE * result;
    room_svr_room_t room = NULL;

    req = &((SVR_ROOM_PKG*)dp_req_data(pkg_body))->data.svr_room_req_delete;

    room = room_svr_room_find(svr, req->room_id);
    if (room == NULL) {
        CPE_ERROR(svr->m_em, "%s: delete: room "FMT_UINT64_T" not exist!", room_svr_name(svr), req->room_id);
        goto DELETE_ROOM_FAIL;
    }

    room_svr_room_notify_room_destoried(room, SVR_ROOM_ROOM_DESTORY_BY_USER);
    room_svr_room_destory(room);

    if (set_pkg_sn(pkg_head) != 0) {
        res_pkg = room_svr_build_response(svr, pkg_body, sizeof(SVR_ROOM_PKG));
        if (res_pkg == NULL) return;
        result = &((SVR_ROOM_PKG *)dp_req_data(res_pkg))->data.svr_room_res_delete;
        result->result = 0;
        room_svr_send_pkg(svr, res_pkg);
    }

    return;

DELETE_ROOM_FAIL:
    if (set_pkg_sn(pkg_head) != 0) {
        res_pkg = room_svr_build_response(svr, pkg_body, sizeof(SVR_ROOM_PKG));
        if (res_pkg == NULL) return;
        result = &((SVR_ROOM_PKG *)dp_req_data(res_pkg))->data.svr_room_res_delete;
        result->result = -1;
        room_svr_send_pkg(svr, res_pkg);
    }
}
