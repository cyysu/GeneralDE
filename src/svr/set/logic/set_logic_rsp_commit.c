#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_manage.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_data.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "set_logic_rsp_ops.h"
#include "protocol/set/logic/set_logic_rsp_carry_info.h"

static dp_req_t set_logic_rsp_commit_build_response(set_logic_rsp_t rsp, logic_context_t op_context, SET_LOGIC_CARRY_INFO * bpg_private, error_monitor_t em);
static int set_logic_rsp_commit_build_carry_data(set_logic_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, error_monitor_t em);

void set_logic_rsp_commit(logic_context_t op_context, void * user_data) {
    set_logic_rsp_t set_logic_rsp;
    set_logic_rsp_manage_t rsp_mgr;
    error_monitor_t em;
    logic_data_t bpg_private_data;
    SET_LOGIC_CARRY_INFO * bpg_private;
    dp_req_t response_buf;

    set_logic_rsp = (set_logic_rsp_t)user_data;
    assert(set_logic_rsp);

    rsp_mgr = set_logic_rsp->m_mgr;
    assert(rsp_mgr);

    em = rsp_mgr->m_em;

    bpg_private_data = logic_context_data_find(op_context, "set_logic_carry_info");
    if (bpg_private_data == NULL) {
        CPE_ERROR(
            em, "%s.%s: set_logic_rsp_commit: no set_logic_carry_info in context!",
            set_logic_rsp_manage_name(rsp_mgr), set_logic_rsp_name(set_logic_rsp));
        set_logic_rsp_manage_free_context(rsp_mgr, op_context);
        return;
    }

    bpg_private = (SET_LOGIC_CARRY_INFO *)logic_data_data(bpg_private_data);
    if (bpg_private->response == 0) {
        if (rsp_mgr->m_debug >= 2) {
            CPE_INFO(
                em, "%s.%s: set_logic_rsp_commit: ignore send response!",
                set_logic_rsp_manage_name(rsp_mgr), set_logic_rsp_name(set_logic_rsp));
        }
        set_logic_rsp_manage_free_context(rsp_mgr, op_context);
        return;
    }

    response_buf = set_logic_rsp_commit_build_response(set_logic_rsp, op_context, bpg_private, em);
    if (response_buf == NULL) {
        goto SEND_ERROR_RESPONSE;
    }

    if (set_logic_rsp_commit_build_carry_data(set_logic_rsp, op_context, response_buf, em) != 0) {
        goto SEND_ERROR_RESPONSE;
    }

    if (dp_dispatch_by_string(rsp_mgr->m_commit_to, response_buf, em) != 0) {
        CPE_ERROR(em, "%s.%s: set_logic_rsp_commit: dispatch fail!", set_logic_rsp_manage_name(rsp_mgr), set_logic_rsp_name(set_logic_rsp));
        goto SEND_ERROR_RESPONSE;
    }

    set_logic_rsp_manage_free_context(rsp_mgr, op_context);
    return;

SEND_ERROR_RESPONSE:
    /* dp_req_child_clear(response_buf); */

    /* set_pkg_init(response_head); */
    /* set_pkg_set_sn(response_head, bpg_private->sn); */
    /* set_pkg_set_category(response_head, set_pkg_response); */
    /* set_pkg_set_to_svr(response_head, bpg_private->from_svr_type, bpg_private->from_svr_id); */
    //TODO: bpg_pkg_set_errno(response_buf, -1);

    /* if (bpg_pkg_dsp_dispatch(rsp_mgr->m_commit_to, response_buf, em) != 0) { */
    /*     CPE_ERROR(em, "%s.%s: set_logic_rsp_commit: send error response fail!", set_logic_rsp_manage_name(rsp_mgr), set_logic_rsp_name(set_logic_rsp)); */
    /*     set_logic_rsp_manage_free_context(rsp_mgr, op_context); */
    /*     return; */
    /* } */

    set_logic_rsp_manage_free_context(rsp_mgr, op_context);
    return;
}

static dp_req_t set_logic_rsp_commit_build_response(
    set_logic_rsp_t rsp, logic_context_t op_context, SET_LOGIC_CARRY_INFO * bpg_private, error_monitor_t em)
{
    set_logic_rsp_manage_t mgr;
    logic_data_t data;
    LPDRMETA data_meta;
    size_t data_size;
    size_t total_size;
    dp_req_t response_buf;
    dp_req_t response_head;

    mgr = rsp->m_mgr;

    data_meta = set_svr_svr_info_find_data_meta_by_cmd(mgr->m_svr_type, bpg_private->response);
    if (data_meta == NULL) {
        CPE_ERROR(
            em, "%s.%s: gen response pkg: find data meta of cmd %d error!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), bpg_private->response);
        return NULL;
    }
 
    data = logic_context_data_find(op_context, dr_meta_name(data_meta));
    if (data == NULL) {
        CPE_ERROR(
            em, "%s.%s: gen response pkg: can`t find %s from ctx!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), dr_meta_name(data_meta));
        return NULL;
    }

    data_size = dr_meta_calc_data_len(data_meta, logic_data_data(data), logic_data_capacity(data));
    total_size = data_size + dr_entry_data_start_pos(mgr->m_pkg_data_entry, 0);

    response_buf = set_logic_rsp_manage_rsp_buf(mgr, total_size);
    if (response_buf == NULL) {
        CPE_ERROR(
            em, "%s.%s: gen response buf: response buf is NULL!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
        return NULL;
    }

    dp_req_set_meta(response_buf, mgr->m_pkg_meta);
    dp_req_set_size(response_buf, total_size);

    memcmp(
        ((char*)dp_req_data(response_buf)) + dr_entry_data_start_pos(mgr->m_pkg_data_entry, 0),
        logic_data_data(data), data_size);

    if (dr_entry_set_from_uint32(
            ((char*)dp_req_data(response_buf)) + dr_entry_data_start_pos(mgr->m_pkg_cmd_entry, 0),
            (uint32_t)bpg_private->response, mgr->m_pkg_cmd_entry, em) != 0)
    {
        CPE_ERROR(
            em, "%s.%s: gen response buf: set cmd fail!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
        return NULL;
    }

    response_head = set_pkg_head_check_create(response_buf);
    if (response_head == NULL) {
        CPE_ERROR(
            em, "%s.%s: gen response buf: response head is NULL!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
        return NULL;
    }

    set_pkg_init(response_head);
    set_pkg_set_sn(response_head, bpg_private->sn);
    set_pkg_set_category(response_head, set_pkg_response);
    set_pkg_set_to_svr(response_head, bpg_private->from_svr_type, bpg_private->from_svr_id);

    if (rsp->m_mgr->m_debug >= 2) {
        CPE_INFO(
            em, "%s.%s: gen response pkg: meta = %s, wirte-size=%d!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), dr_meta_name(mgr->m_pkg_meta), (int)total_size);
    }
            
    return response_buf;
}

void set_logic_rsp_manage_free_context(set_logic_rsp_manage_t mgr, logic_context_t op_context) {
    if (mgr->m_ctx_fini) {
        mgr->m_ctx_fini(op_context, mgr->m_ctx_ctx);
    }
    logic_context_free(op_context);
}

static int set_logic_rsp_commit_build_carry_data(set_logic_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, error_monitor_t em) {
    logic_data_t carry_info_list_data;
    SET_LOGIC_CARRY_METAS * carry_info_list;
    uint32_t i;
    struct dp_req_it child_it;
    dp_req_t child_data;

    carry_info_list_data = logic_context_data_find(op_context, "set_logic_carry_metas");
    if (carry_info_list_data == NULL) return 0;

    carry_info_list = logic_data_data(carry_info_list_data);
    dp_req_childs(pkg, &child_it);

    for(i = 0, child_data = dp_req_next(&child_it); i < carry_info_list->count; ++i, child_data = dp_req_next(&child_it)) {
        logic_data_t carry_data = logic_context_data_find(op_context, carry_info_list->data[i].name);

        if (carry_data == NULL) {
            CPE_ERROR(
                em, "%s.%s: copy_carry data %s: data not exist!",
                set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), carry_info_list->data[i].name);
            return -1;
        }

        /* if (child_data && dp_req_is_type(child_data, req_type_set_pkg)) { */
        /*     child_data = dp_req_next(&child_it); */
        /* } */

        if (child_data == NULL) {
            child_data = dp_req_create(dp_req_mgr(pkg), 0);
            if (child_data == NULL) {
                CPE_ERROR(
                    em, "%s.%s: copy_carry data %s: create child_data fial!",
                    set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), carry_info_list->data[i].name);
                return -1;
            }
            else {
                dp_req_add_to_parent(child_data, pkg);
            }
        }

        dp_req_set_buf(child_data, logic_data_data(carry_data), logic_data_capacity(carry_data));
        dp_req_set_size(child_data, logic_data_capacity(carry_data));
        dp_req_set_meta(child_data, logic_data_meta(carry_data));
    }

    /* while(child_data) { */
    /*     dp_req_t next = dp_req_next(&child_it); */

    /*     if (!dp_req_is_type(child_data, req_type_bpg_pkg)) { */
    /*         if (dp_req_manage_by_parent(child_data)) { */
    /*             dp_req_free(child_data); */
    /*         } */
    /*         else { */
    /*             dp_req_set_parent(child_data, NULL); */
    /*         } */
    /*     } */

    /*     child_data = next; */
    /* } */

    return 0;
}
