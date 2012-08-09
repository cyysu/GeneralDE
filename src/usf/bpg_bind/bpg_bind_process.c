#include "cpe/utils/error.h"
#include "cpe/dp/dp_manage.h"
#include "gd/vnet/vnet_control_pkg.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_bind/bpg_bind_manage.h"
#include "bpg_bind_internal_ops.h"

int bpg_bind_manage_outgoing_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    bpg_bind_manage_t mgr = (bpg_bind_manage_t)ctx;
    bpg_pkg_t pkg;

    pkg = bpg_pkg_from_dp_req(req);
    if (pkg == NULL) {
        return bpg_pkg_dsp_pass(mgr->m_outgoing_send_to, req, em);
    }

    return bpg_pkg_dsp_dispatch(mgr->m_outgoing_send_to, pkg, em);
}

static void bpg_bind_manage_kickoff_old(
    bpg_bind_manage_t mgr,
    uint32_t new_client_id,
    uint32_t old_connection_id,
    uint32_t old_client_id,
    error_monitor_t em)
{
    bpg_pkg_t kickoff_pkg;
    vnet_control_pkg_t control_pkg;

    kickoff_pkg = bpg_bind_manage_data_pkg(mgr);
    if (kickoff_pkg == NULL) {
        CPE_ERROR(mgr->m_em, "bpg_bind_manage_kickoff_old: get data pkg fail!");
    }
    else {
        bpg_pkg_init(kickoff_pkg);
        bpg_pkg_set_connection_id(kickoff_pkg, old_connection_id);
        bpg_pkg_set_cmd(kickoff_pkg, 10413);
        bpg_pkg_set_client_id(kickoff_pkg, old_client_id);

        if (bpg_pkg_dsp_dispatch(mgr->m_outgoing_send_to, kickoff_pkg, em) != 0) {
            CPE_ERROR(mgr->m_em, "bpg_bind_manage_kickoff_old: send data pkg fail!");
        }
    }

    control_pkg = bpg_bind_manage_control_pkg(mgr);
    if (kickoff_pkg == NULL) {
        CPE_ERROR(mgr->m_em, "bpg_bind_manage_kickoff_old: send control pkg fail!");
    }
    else {
        vnet_control_pkg_init(control_pkg);
        vnet_control_pkg_set_cmd(control_pkg, vnet_control_op_disconnect);
        vnet_control_pkg_set_connection_id(control_pkg, old_connection_id);
        if (bpg_pkg_dsp_pass(mgr->m_outgoing_send_to, vnet_control_pkg_to_dp_req(control_pkg), em) != 0) {
            CPE_ERROR(mgr->m_em, "bpg_bind_manage_kickoff_old: send control pkg fail!");
        }
    }
}

int bpg_bind_manage_incoming_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    bpg_bind_manage_t mgr = (bpg_bind_manage_t)ctx;
    bpg_pkg_t pkg;
    struct bpg_bind_binding * found_binding;
    uint32_t connection_id;
    uint32_t client_id;


    pkg = bpg_pkg_from_dp_req(req);
    if (pkg == NULL) {
        return bpg_pkg_dsp_pass(mgr->m_outgoing_send_to, req, em);
    }

    connection_id = bpg_pkg_connection_id(pkg);
    client_id = bpg_pkg_client_id(pkg);

    if (client_id == 0) {
        CPE_ERROR(
            mgr->m_em, "%s: dispatch cmd %d: client id invalid!",
            bpg_bind_manage_name(mgr), bpg_pkg_cmd(pkg));
        return -1;
    }

    if (connection_id != BPG_INVALID_CONNECTION_ID) { 
        CPE_ERROR(
            mgr->m_em, "%s: dispatch cmd %d: connection id invalid!",
            bpg_bind_manage_name(mgr), bpg_pkg_cmd(pkg));
        return -1;
    }

    found_binding = bpg_bind_binding_find_by_client_id(mgr, client_id);
    if (found_binding) {
        if (found_binding->m_connection_id != connection_id) {
            bpg_bind_manage_kickoff_old(
                mgr, client_id, found_binding->m_connection_id, found_binding->m_client_id, mgr->m_em);
            bpg_bind_binding_free(mgr, found_binding);
        }
    }

    found_binding = bpg_bind_binding_find_by_connection_id(mgr, connection_id); 
    if (found_binding) {
        bpg_bind_binding_free(mgr, found_binding);
    }

    if (bpg_bind_binding_create(mgr, client_id, connection_id) != 0) { 
        CPE_ERROR( 
            mgr->m_em, "%s: ep %d: binding: create binding fail!", 
            bpg_bind_manage_name(mgr), (int)connection_id); 
    } 

    if (bpg_pkg_dsp_dispatch(mgr->m_incoming_send_to, pkg, em) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: dispatch cmd %d error!",
            bpg_bind_manage_name(mgr), bpg_pkg_cmd(pkg));
        return -1;
    }

    return 0;
}
