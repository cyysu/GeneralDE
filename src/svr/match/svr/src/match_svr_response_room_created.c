#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "svr/center/agent/center_agent.h"
#include "svr/set/share/set_pkg.h"
#include "protocol/svr/room/svr_room_pro.h"
#include "match_svr_ops.h"

void match_svr_response_room_created(match_svr_t svr, dp_req_t agent_pkg) {
    SVR_ROOM_RES_CREATE * response;
    match_svr_room_t room = NULL;
    SVR_MATCH_ROOM_META * room_meta;

    response = & ((SVR_ROOM_PKG *)dp_req_data(agent_pkg))->data.svr_room_res_create;

    if (response->result != 0) {
        CPE_ERROR(
            svr->m_em, "%s: room(creating_id=%d): response_created: create fail, result=%d",
            match_svr_name(svr), set_pkg_sn(agent_pkg), response->result);
        return;
    } 

    room = match_svr_room_find_creating(svr, set_pkg_sn(agent_pkg));
    if (room == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: room(creating_id=%d): response_created: room not exist",
            match_svr_name(svr), set_pkg_sn(agent_pkg));
        match_svr_room_send_delete_req(svr, set_pkg_sn(agent_pkg), response->room_id);
        return;
    }
    
    room_meta = match_svr_meta_foom_find(svr, room->m_data->match_room_type);
    if (room_meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: room(creating_id=%d, room_id="FMT_UINT64_T"): response_created: room type %d is unknown",
            match_svr_name(svr), set_pkg_sn(agent_pkg), room->m_data->match_room_id, room->m_data->match_room_type);
        match_svr_room_send_delete_req(svr, set_pkg_sn(agent_pkg), response->room_id);
        return;
    }

    match_svr_room_notify_room_created(room, set_pkg_from_svr_id(agent_pkg), response->room_id);

    match_svr_room_destory(room);
}

