#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/net/net_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "set_svr_ops.h"
#include "set_svr_center_ops.h"
#include "set_svr_router_ops.h"

extern char g_metalib_svr_set_pro[];
static void set_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_set_svr = {
    "svr_set_svr",
    set_svr_clear
};

set_svr_t
set_svr_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct set_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct set_svr));
    if (svr_node == NULL) return NULL;

    svr = (set_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_debug = 0;
    svr->m_ringbuf = NULL;
    svr->m_local_search_timer_id = GD_TIMER_ID_INVALID;
    svr->m_ev_loop = net_mgr_ev_loop(gd_app_net_mgr(app));
    svr->m_incoming_buf = NULL;

    svr->m_max_conn_id = 0;
    svr->m_router_ip[0] = 0;
    svr->m_router_port = 0;
    svr->m_router_conn_timeout_ms = 30000;
    svr->m_router_process_count_per_tick = 10;

    if (cpe_hash_table_init(
            &svr->m_svr_types_by_id,
            alloc,
            (cpe_hash_fun_t) set_svr_svr_type_hash_by_id,
            (cpe_hash_cmp_t) set_svr_svr_type_eq_by_id,
            CPE_HASH_OBJ2ENTRY(set_svr_svr_type, m_hh_by_id),
            -1) != 0)
    {
        nm_node_free(svr_node);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &svr->m_svr_types_by_name,
            alloc,
            (cpe_hash_fun_t) set_svr_svr_type_hash_by_name,
            (cpe_hash_cmp_t) set_svr_svr_type_eq_by_name,
            CPE_HASH_OBJ2ENTRY(set_svr_svr_type, m_hh_by_name),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_svr_types_by_id);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_svrs,
            alloc,
            (cpe_hash_fun_t) set_svr_svr_hash,
            (cpe_hash_cmp_t) set_svr_svr_eq,
            CPE_HASH_OBJ2ENTRY(set_svr_svr, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_svr_types_by_id);
        cpe_hash_table_fini(&svr->m_svr_types_by_name);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_routers_by_id,
            alloc,
            (cpe_hash_fun_t) set_svr_router_hash_by_id,
            (cpe_hash_cmp_t) set_svr_router_eq_by_id,
            CPE_HASH_OBJ2ENTRY(set_svr_router, m_hh_by_id),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_svr_types_by_id);
        cpe_hash_table_fini(&svr->m_svr_types_by_name);
        cpe_hash_table_fini(&svr->m_svrs);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_routers_by_addr,
            alloc,
            (cpe_hash_fun_t) set_svr_router_hash_by_addr,
            (cpe_hash_cmp_t) set_svr_router_eq_by_addr,
            CPE_HASH_OBJ2ENTRY(set_svr_router, m_hh_by_addr),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_svr_types_by_id);
        cpe_hash_table_fini(&svr->m_svr_types_by_name);
        cpe_hash_table_fini(&svr->m_svrs);
        cpe_hash_table_fini(&svr->m_routers_by_id);
        nm_node_free(svr_node);
        return NULL;
    }

    svr->m_local_svr_count = 0;
    TAILQ_INIT(&svr->m_local_svrs);
    TAILQ_INIT(&svr->m_accept_router_conns);

    svr->m_center = set_svr_center_create(svr);
    if (svr->m_center == NULL) {
        cpe_hash_table_fini(&svr->m_svr_types_by_id);
        cpe_hash_table_fini(&svr->m_svr_types_by_name);
        cpe_hash_table_fini(&svr->m_svrs);
        cpe_hash_table_fini(&svr->m_routers_by_id);
        cpe_hash_table_fini(&svr->m_routers_by_addr);
        nm_node_free(svr_node);
        return NULL;
    }

    svr->m_router_conn_fsm_def = set_svr_router_conn_create_fsm_def("router_conn_fsm", alloc, em);
    if (svr->m_router_conn_fsm_def == NULL) {
        CPE_ERROR(em, "%s: create: create router_conn_fsm_def fail!", name);
        cpe_hash_table_fini(&svr->m_svr_types_by_id);
        cpe_hash_table_fini(&svr->m_svr_types_by_name);
        cpe_hash_table_fini(&svr->m_svrs);
        cpe_hash_table_fini(&svr->m_routers_by_id);
        cpe_hash_table_fini(&svr->m_routers_by_addr);
        set_svr_center_free(svr->m_center);
        nm_node_free(svr_node);
        return NULL;
    }

    if (gd_app_tick_add(app, set_svr_dispatch_tick, svr, 0) != 0) {
        CPE_ERROR(em, "%s: create: add dispatch tick fail!", name);
        cpe_hash_table_fini(&svr->m_svr_types_by_id);
        cpe_hash_table_fini(&svr->m_svr_types_by_name);
        cpe_hash_table_fini(&svr->m_svrs);
        cpe_hash_table_fini(&svr->m_routers_by_id);
        cpe_hash_table_fini(&svr->m_routers_by_addr);
        set_svr_center_free(svr->m_center);
        fsm_def_machine_free(svr->m_router_conn_fsm_def);
        nm_node_free(svr_node);
        return NULL;
    }

    mem_buffer_init(&svr->m_dump_buffer_head, svr->m_alloc);
    mem_buffer_init(&svr->m_dump_buffer_carry, svr->m_alloc);
    mem_buffer_init(&svr->m_dump_buffer_body, svr->m_alloc);

    nm_node_set_type(svr_node, &s_nm_node_type_set_svr);

    return svr;
}

static void set_svr_clear(nm_node_t node) {
    set_svr_t svr;
    svr = (set_svr_t)nm_node_data(node);

    set_svr_stop_local_search_timer(svr);
    gd_app_tick_remove(svr->m_app, set_svr_dispatch_tick, svr);

    while(!TAILQ_EMPTY(&svr->m_accept_router_conns)) {
        set_svr_router_conn_free(TAILQ_FIRST(&svr->m_accept_router_conns));
    }

    set_svr_router_free_all(svr);
    cpe_hash_table_fini(&svr->m_routers_by_id);
    cpe_hash_table_fini(&svr->m_routers_by_addr);

    set_svr_svr_free_all(svr);
    cpe_hash_table_fini(&svr->m_svrs);

    set_svr_svr_type_free_all(svr);
    cpe_hash_table_fini(&svr->m_svr_types_by_id);
    cpe_hash_table_fini(&svr->m_svr_types_by_name);

    if (svr->m_center) {
        set_svr_center_free(svr->m_center);
        svr->m_center = NULL;
    }

    if (svr->m_ringbuf) {
        ringbuffer_delete(svr->m_ringbuf);
        svr->m_ringbuf = NULL;
    }

    if (svr->m_incoming_buf) {
        dp_req_free(svr->m_incoming_buf);
        svr->m_incoming_buf = NULL;
    }

    if (svr->m_router_conn_fsm_def) {
        fsm_def_machine_free(svr->m_router_conn_fsm_def);
        svr->m_router_conn_fsm_def = NULL;
    }

    mem_buffer_clear(&svr->m_dump_buffer_head);
    mem_buffer_clear(&svr->m_dump_buffer_carry);
    mem_buffer_clear(&svr->m_dump_buffer_body);
}

int set_svr_set_ringbuf_size(set_svr_t svr, size_t capacity) {
    assert(svr->m_ringbuf == NULL);
    svr->m_ringbuf = ringbuffer_new(capacity);
    if (svr->m_ringbuf == NULL) return -1;
    return 0;
}

void set_svr_do_local_search(void * ctx, gd_timer_id_t timer_id, void * arg) {
    set_svr_t svr = ctx;
    if (set_svr_svr_search(svr) != 0) {
        CPE_ERROR(svr->m_em, "%s: local search: fail!", set_svr_name(svr));
    }
    else {
        if (svr->m_debug >= 2) {
            CPE_ERROR(svr->m_em, "%s: local search: complete!", set_svr_name(svr));
        }
    }
}

int set_svr_start_local_search_timer(set_svr_t svr, tl_time_span_t span) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(svr->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: start local search timer: get default timer manager fail!", set_svr_name(svr));
        return -1;
    }

    assert(svr->m_local_search_timer_id == GD_TIMER_ID_INVALID);

    if (gd_timer_mgr_regist_timer(timer_mgr, &svr->m_local_search_timer_id, set_svr_do_local_search, svr, NULL, NULL, 0, span, -1) != 0) {
        assert(svr->m_local_search_timer_id == GD_TIMER_ID_INVALID);
        CPE_ERROR(svr->m_em, "%s: start local search timer: regist timer fail!", set_svr_name(svr));
        return -1;
    }

    assert(svr->m_local_search_timer_id != GD_TIMER_ID_INVALID);
    return 0;
}

void set_svr_stop_local_search_timer(set_svr_t svr) {
    gd_timer_mgr_t timer_mgr;

    if (svr->m_local_search_timer_id == GD_TIMER_ID_INVALID) return;

    timer_mgr = gd_timer_mgr_default(svr->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: stop local search timer: get default timer manager fail!", set_svr_name(svr));
        return;
    }

    gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_local_search_timer_id);
    svr->m_local_search_timer_id = GD_TIMER_ID_INVALID;
}

gd_app_context_t set_svr_app(set_svr_t svr) {
    return svr->m_app;
}

void set_svr_free(set_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_set_svr) return;
    nm_node_free(svr_node);
}

set_svr_t
set_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_set_svr) return NULL;
    return (set_svr_t)nm_node_data(node);
}

set_svr_t
set_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_set_svr) return NULL;
    return (set_svr_t)nm_node_data(node);
}

const char * set_svr_name(set_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

ringbuffer_block_t set_svr_ringbuffer_alloc(set_svr_t svr, int size, uint32_t id) {
    ringbuffer_block_t blk;

TRY_AGAIN:
    blk = ringbuffer_alloc(svr->m_ringbuf , size);
    if (blk == NULL) {
        int collect_id = ringbuffer_collect(svr->m_ringbuf);
        if(collect_id < 0) {
            CPE_ERROR(svr->m_em, "%s: ringbuffer_collect: not enouth capacity, len=%d!", set_svr_name(svr), size);
            return NULL;
        }

        if (collect_id == id) {
            CPE_ERROR(svr->m_em, "%s: ringbuffer_collect: self make ringbuffer full, id=%d!", set_svr_name(svr), id);
            return NULL;
        }
        else if (collect_id == svr->m_center->m_conn_id) {
            CPE_ERROR(svr->m_em, "%s: ringbuffer_collect: center make ringbuffer full!", set_svr_name(svr));
            set_svr_center_apply_evt(svr->m_center, set_svr_center_fsm_evt_disconnected);
        }
        else {
            set_svr_router_t router = set_svr_router_find_by_id(svr, collect_id);
            assert(router);

            CPE_ERROR(
                svr->m_em, "%s: ringbuffer_collect: router %d-%d.%d make ringbuffer full!",
                set_svr_name(svr), router->m_id, router->m_ip, router->m_port);

            set_svr_router_clear_data(router);

            goto TRY_AGAIN;
        }
    }

    return blk;
}
