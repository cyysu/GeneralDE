#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/net/net_listener.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "match_svr_ops.h"

extern char g_metalib_svr_match_pro[];
extern char g_metalib_svr_room_pro[];
static void match_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_match_svr = {
    "svr_match_svr",
    match_svr_clear
};

match_svr_t
match_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    uint16_t room_svr_type_id,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct match_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct match_svr));
    if (svr_node == NULL) return NULL;

    svr = (match_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_debug = 0;
    svr->m_room_svr_type_id = room_svr_type_id;
    svr->m_create_retry_span_s = 5;
    svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    svr->m_outgoing_pkg = NULL;
    svr->m_send_to = NULL;
    svr->m_match_require_recv_at = NULL;
    svr->m_room_response_recv_at = NULL;
    svr->m_meta_count = 0;
    svr->m_metas = NULL;
    svr->m_creating_max_id = 0;

    svr->m_room_meta_req_create_room =
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_room_pro, "svr_room_req_create");
    assert(svr->m_room_meta_req_create_room);

    svr->m_room_meta_req_delete_room =
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_room_pro, "svr_room_req_delete");
    assert(svr->m_room_meta_req_delete_room);

    svr->m_room_data_meta =
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_match_pro, "svr_match_room_record");
    assert(svr->m_room_data_meta);
    svr->m_room_data_mgr = NULL;
    svr->m_room_data_buf = NULL;

    svr->m_user_data_meta = 
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_match_pro, "svr_match_user_record");
    svr->m_user_data_mgr = NULL;
    svr->m_user_data_buf = NULL;

    if (cpe_hash_table_init(
            &svr->m_matching_rooms,
            alloc,
            (cpe_hash_fun_t) match_svr_room_room_id_hash,
            (cpe_hash_cmp_t) match_svr_room_room_id_eq,
            CPE_HASH_OBJ2ENTRY(match_svr_room, m_hh),
            -1) != 0)
    {
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_users,
            alloc,
            (cpe_hash_fun_t) match_svr_user_hash,
            (cpe_hash_cmp_t) match_svr_user_eq,
            CPE_HASH_OBJ2ENTRY(match_svr_user, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_matching_rooms);
        nm_node_free(svr_node);
        return NULL;
    }

    nm_node_set_type(svr_node, &s_nm_node_type_match_svr);

    return svr;
}

static void match_svr_clear(nm_node_t node) {
    match_svr_t svr;
    svr = (match_svr_t)nm_node_data(node);

    match_svr_room_free_all(svr);
    match_svr_user_free_all(svr);

    if (svr->m_room_data_mgr) {
        aom_obj_mgr_free(svr->m_room_data_mgr);
        svr->m_room_data_mgr = NULL;
    }

    if (svr->m_room_data_buf) {
        mem_free(svr->m_alloc, svr->m_room_data_buf);
        svr->m_room_data_buf = NULL;
    }

    if (svr->m_user_data_mgr) {
        aom_obj_mgr_free(svr->m_user_data_mgr);
        svr->m_user_data_mgr = NULL;
    }

    if (svr->m_user_data_buf) {
        mem_free(svr->m_alloc, svr->m_user_data_buf);
        svr->m_user_data_buf = NULL;
    }

    if (svr->m_outgoing_pkg) {
        dp_req_free(svr->m_outgoing_pkg);
        svr->m_outgoing_pkg = NULL;
    }

    if (svr->m_send_to) {
        mem_free(svr->m_alloc, svr->m_send_to);
        svr->m_send_to = NULL;
    }

    if (svr->m_match_require_recv_at != NULL) {
        dp_rsp_free(svr->m_match_require_recv_at);
        svr->m_match_require_recv_at = NULL;
    }

    if (svr->m_room_response_recv_at != NULL) {
        dp_rsp_free(svr->m_room_response_recv_at);
        svr->m_room_response_recv_at = NULL;
    }

    if (svr->m_metas) {
        mem_free(svr->m_alloc, svr->m_metas);
        svr->m_metas = NULL;
        svr->m_meta_count = 0;
    }

    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);
        assert(timer_mgr);
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    cpe_hash_table_fini(&svr->m_users);
    cpe_hash_table_fini(&svr->m_matching_rooms);
}

void match_svr_free(match_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_match_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t match_svr_app(match_svr_t svr) {
    return svr->m_app;
}

match_svr_t
match_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_match_svr) return NULL;
    return (match_svr_t)nm_node_data(node);
}

match_svr_t
match_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_match_svr) return NULL;
    return (match_svr_t)nm_node_data(node);
}

const char * match_svr_name(match_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
match_svr_name_hs(match_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t match_svr_cur_time(match_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int match_svr_set_send_to(match_svr_t svr, const char * send_to) {
    cpe_hash_string_t new_send_to = cpe_hs_create(svr->m_alloc, send_to);
    if (new_send_to == NULL) return -1;

    if (svr->m_send_to) mem_free(svr->m_alloc, svr->m_send_to);
    svr->m_send_to = new_send_to;

    return 0;
}

int match_svr_match_require_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int match_svr_set_match_require_recv_at(match_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_match_require_recv_at != NULL) dp_rsp_free(svr->m_match_require_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.match.require", match_svr_name(svr));
    svr->m_match_require_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_match_require_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: match_svr_set_recv_at: create rsp fail!",
            match_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_match_require_recv_at, match_svr_match_require_rsp, svr);

    if (dp_rsp_bind_string(svr->m_match_require_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: match_svr_set_recv_at: bind rsp to %s fail!",
            match_svr_name(svr), name);
        dp_rsp_free(svr->m_match_require_recv_at);
        svr->m_match_require_recv_at = NULL;
        return -1;
    }

    return 0;
}

void match_svr_timer(void * ctx, gd_timer_id_t timer_id, void * arg);
int match_svr_set_check_span(match_svr_t svr, uint32_t span_ms) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);

    if (timer_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: set check span: timer_mgr not exist!", match_svr_name(svr));
        return -1;
    }

    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    if (gd_timer_mgr_regist_timer(timer_mgr, &svr->m_check_timer_id, match_svr_timer, svr, NULL, NULL, span_ms, span_ms, -1) != 0) {
        CPE_ERROR(svr->m_em, "%s: set check span: create timer fail!", match_svr_name(svr));
        return -1;
    }

    return 0;
}

dp_req_t match_svr_pkg_buf(match_svr_t svr, size_t capacity) {
    if (svr->m_outgoing_pkg && dp_req_capacity(svr->m_outgoing_pkg) < capacity) {
        dp_req_free(svr->m_outgoing_pkg);
        svr->m_outgoing_pkg = NULL;
    }

    if (svr->m_outgoing_pkg == NULL) {
        svr->m_outgoing_pkg = dp_req_create(gd_app_dp_mgr(svr->m_app), capacity);
        if (svr->m_outgoing_pkg == NULL) {
            CPE_ERROR(svr->m_em, "%s: crate outgoing pkg buf fail!", match_svr_name(svr));
            return NULL;
        }

        if (set_pkg_head_check_create(svr->m_outgoing_pkg) == NULL) {
            CPE_ERROR(svr->m_em, "%s: crate outgoing pkg buf head fail!", match_svr_name(svr));
            return NULL;
        }
    }

    dp_req_set_meta(svr->m_outgoing_pkg, NULL);

    return svr->m_outgoing_pkg;
}

dp_req_t match_svr_build_response(match_svr_t svr, dp_req_t req, size_t capacity) {
    dp_req_t body = match_svr_pkg_buf(svr, capacity);

    if (body == NULL) return NULL;

    set_pkg_init_response(body, req);
    dp_req_set_size(body, capacity);

    ((SVR_MATCH_PKG*)dp_req_data(body))->cmd
        = ((SVR_MATCH_PKG*)dp_req_data(req))->cmd + 1;

    return body;
}

dp_req_t
match_svr_build_notify(match_svr_t svr, uint32_t cmd, size_t capacity) {
    dp_req_t body = match_svr_pkg_buf(svr, capacity);
    dp_req_t head;

    if (body == NULL) return NULL;

    head = set_pkg_head_find(body);
    assert(head);

    set_pkg_init(head);
    set_pkg_set_sn(head, 0);
    set_pkg_set_category(head, set_pkg_notify);

    dp_req_set_size(body, capacity);

    ((SVR_MATCH_PKG*)dp_req_data(body))->cmd = cmd;

    return body;
}

int match_svr_send_pkg(match_svr_t svr, dp_req_t req) {
    if (dp_dispatch_by_string(svr->m_send_to, req, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: send pkg fail!", match_svr_name(svr));
        return -1;
    }

    return 0;
}

int match_svr_room_data_init_from_mem(match_svr_t svr, size_t memory_size) {
    if (svr->m_room_data_mgr) {
        aom_obj_mgr_free(svr->m_room_data_mgr);
        svr->m_room_data_mgr = NULL;
    }

    if (svr->m_room_data_buf) {
        mem_free(svr->m_alloc, svr->m_room_data_buf);
        svr->m_room_data_buf = NULL;
    }
    
    svr->m_room_data_buf = mem_alloc(svr->m_alloc, memory_size);
    if (svr->m_room_data_buf == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: init room data from mem: alloc buf fail, size=%d!",
            match_svr_name(svr), (int)memory_size);
        return -1;
    }

    if (aom_obj_mgr_buf_init(svr->m_room_data_meta, svr->m_room_data_buf, memory_size, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em,  "%s: init room data from mem: init buf fail!", match_svr_name(svr));
        return -1;
    }

    svr->m_room_data_mgr = aom_obj_mgr_create(svr->m_alloc, svr->m_room_data_buf, memory_size, svr->m_em);
    if (svr->m_room_data_mgr == NULL) {
        CPE_ERROR(svr->m_em,  "%s: init room data from mem: create aom obj mgr fail!", match_svr_name(svr));
        return -1;
    }

    return 0;
}

int match_svr_room_data_init_from_shm(match_svr_t svr, int shm_key) {
    cpe_shm_id_t shmid;
    cpe_shmid_ds shm_info;
    void * data;

    if (svr->m_room_data_mgr) {
        aom_obj_mgr_free(svr->m_room_data_mgr);
        svr->m_room_data_mgr = NULL;
    }

    if (svr->m_room_data_buf) {
        mem_free(svr->m_alloc, svr->m_room_data_buf);
        svr->m_room_data_buf = NULL;
    }
    
    shmid = cpe_shm_get(shm_key);
    if (shmid == -1) {
        CPE_ERROR(
            svr->m_em, "%s: init room data from shm: get shm (key=%d) fail, errno=%d (%s)",
            match_svr_name(svr), shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    if (cpe_shm_ds_get(shmid, &shm_info) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: init room data from shm: get shm info (key=%d) fail, errno=%d (%s)",
            match_svr_name(svr), shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    data = cpe_shm_attach(shmid, NULL, 0);
    if (data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: init room data from shm: attach shm (key=%d, size=%d) fail, errno=%d (%s)",
            match_svr_name(svr), shm_key, shmid, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    svr->m_room_data_mgr = aom_obj_mgr_create(svr->m_alloc, data, shm_info.shm_segsz, svr->m_em);
    if (svr->m_room_data_mgr == NULL) {
        cpe_shm_detach(data);
        CPE_ERROR(svr->m_em, "%s: init room data from shm: create grp obj mgr (from shm) fail!", match_svr_name(svr));
        return -1;
    }

    if (!dr_meta_compatible(svr->m_room_data_meta, aom_obj_mgr_meta(svr->m_room_data_mgr))) {
        cpe_shm_detach(data);
        CPE_ERROR(svr->m_em, "%s: init room data from shm: aom grp meta not compatable!", match_svr_name(svr));
        return -1;
    }

    return 0;
}

int match_svr_user_data_init_from_mem(match_svr_t svr, size_t memory_size) {
    if (svr->m_user_data_mgr) {
        aom_obj_mgr_free(svr->m_user_data_mgr);
        svr->m_user_data_mgr = NULL;
    }

    if (svr->m_user_data_buf) {
        mem_free(svr->m_alloc, svr->m_user_data_buf);
        svr->m_user_data_buf = NULL;
    }
    
    svr->m_user_data_buf = mem_alloc(svr->m_alloc, memory_size);
    if (svr->m_user_data_buf == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: init user data from mem: alloc buf fail, size=%d!",
            match_svr_name(svr), (int)memory_size);
        return -1;
    }

    if (aom_obj_mgr_buf_init(svr->m_user_data_meta, svr->m_user_data_buf, memory_size, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em,  "%s: init user data from mem: init buf fail!", match_svr_name(svr));
        return -1;
    }

    svr->m_user_data_mgr = aom_obj_mgr_create(svr->m_alloc, svr->m_user_data_buf, memory_size, svr->m_em);
    if (svr->m_user_data_mgr == NULL) {
        CPE_ERROR(svr->m_em,  "%s: init user data from mem: create aom obj mgr fail!", match_svr_name(svr));
        return -1;
    }

    return 0;
}

int match_svr_user_data_init_from_shm(match_svr_t svr, int shm_key) {
    cpe_shm_id_t shmid;
    cpe_shmid_ds shm_info;
    void * data;

    if (svr->m_user_data_mgr) {
        aom_obj_mgr_free(svr->m_user_data_mgr);
        svr->m_user_data_mgr = NULL;
    }

    if (svr->m_user_data_buf) {
        mem_free(svr->m_alloc, svr->m_user_data_buf);
        svr->m_user_data_buf = NULL;
    }
    
    shmid = cpe_shm_get(shm_key);
    if (shmid == -1) {
        CPE_ERROR(
            svr->m_em, "%s: init user data from shm: get shm (key=%d) fail, errno=%d (%s)",
            match_svr_name(svr), shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    if (cpe_shm_ds_get(shmid, &shm_info) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: init user data from shm: get shm info (key=%d) fail, errno=%d (%s)",
            match_svr_name(svr), shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    data = cpe_shm_attach(shmid, NULL, 0);
    if (data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: init user data from shm: attach shm (key=%d, size=%d) fail, errno=%d (%s)",
            match_svr_name(svr), shm_key, shmid, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    svr->m_user_data_mgr = aom_obj_mgr_create(svr->m_alloc, data, shm_info.shm_segsz, svr->m_em);
    if (svr->m_user_data_mgr == NULL) {
        cpe_shm_detach(data);
        CPE_ERROR(svr->m_em, "%s: init user data from shm: create grp obj mgr (from shm) fail!", match_svr_name(svr));
        return -1;
    }

    if (!dr_meta_compatible(svr->m_user_data_meta, aom_obj_mgr_meta(svr->m_user_data_mgr))) {
        cpe_shm_detach(data);
        CPE_ERROR(svr->m_em, "%s: init user data from shm: aom grp meta not compatable!", match_svr_name(svr));
        return -1;
    }

    return 0;
}
