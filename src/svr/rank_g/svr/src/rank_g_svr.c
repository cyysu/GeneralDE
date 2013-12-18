#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
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
#include "cpe/aom/aom_obj_hash.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_stub_buff.h"
#include "rank_g_svr_ops.h"

extern char g_metalib_svr_rank_g_pro[];
static void rank_g_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_rank_g_svr = {
    "svr_rank_g_svr",
    rank_g_svr_clear
};

#define RANK_G_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_rank_g_pro, __name); \
    assert(svr-> __arg)

rank_g_svr_t
rank_g_svr_create(gd_app_context_t app, const char * name, set_svr_stub_t stub, mem_allocrator_t alloc, error_monitor_t em) {
    struct rank_g_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct rank_g_svr));
    if (svr_node == NULL) return NULL;

    svr = (rank_g_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_debug = 0;
    svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    svr->m_recv_at = NULL;

    svr->m_index_count = 0;
    bzero(&svr->m_indexs, sizeof(svr->m_indexs));

    RANK_G_SVR_LOAD_META(m_pkg_meta_res_error, "svr_rank_g_res_error");
    RANK_G_SVR_LOAD_META(m_pkg_meta_res_query, "svr_rank_g_res_query");
    RANK_G_SVR_LOAD_META(m_pkg_meta_res_query_with_data, "svr_rank_g_res_query_with_data");

    svr->m_record_meta = NULL;
    svr->m_record_size = 0;
    svr->m_uin_entry = NULL;
    svr->m_uin_start_pos = 0;
    svr->m_record_mgr = NULL;
    svr->m_record_hash = NULL;

    nm_node_set_type(svr_node, &s_nm_node_type_rank_g_svr);

    return svr;
}

static void rank_g_svr_clear(nm_node_t node) {
    rank_g_svr_t svr;
    svr = (rank_g_svr_t)nm_node_data(node);

    if (svr->m_recv_at != NULL) {
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
    }

    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);
        assert(timer_mgr);
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    /*record*/
    if (svr->m_record_mgr) {
        aom_obj_mgr_free(svr->m_record_mgr);
        svr->m_record_mgr = NULL;
    }

    if (svr->m_record_hash) {
        aom_obj_hash_table_free(svr->m_record_hash);
        svr->m_record_hash = NULL;
    }
}

void rank_g_svr_free(rank_g_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_rank_g_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t rank_g_svr_app(rank_g_svr_t svr) {
    return svr->m_app;
}

rank_g_svr_t
rank_g_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_rank_g_svr) return NULL;
    return (rank_g_svr_t)nm_node_data(node);
}

rank_g_svr_t
rank_g_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_rank_g_svr) return NULL;
    return (rank_g_svr_t)nm_node_data(node);
}

const char * rank_g_svr_name(rank_g_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
rank_g_svr_name_hs(rank_g_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t rank_g_svr_cur_time(rank_g_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int rank_g_svr_request_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int rank_g_svr_set_request_recv_at(rank_g_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_recv_at != NULL) dp_rsp_free(svr->m_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.rank_g.request", rank_g_svr_name(svr));
    svr->m_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: rank_g_svr_set_recv_at: create rsp fail!",
            rank_g_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_recv_at, rank_g_svr_request_rsp, svr);

    if (dp_rsp_bind_string(svr->m_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: rank_g_svr_set_recv_at: bind rsp to %s fail!",
            rank_g_svr_name(svr), name);
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
        return -1;
    }

    return 0;
}

void rank_g_svr_timer(void * ctx, gd_timer_id_t timer_id, void * arg);
int rank_g_svr_set_check_span(rank_g_svr_t svr, uint32_t span_ms) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);

    if (timer_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: set check span: timer_mgr not exist!", rank_g_svr_name(svr));
        return -1;
    }

    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    if (gd_timer_mgr_regist_timer(timer_mgr, &svr->m_check_timer_id, rank_g_svr_timer, svr, NULL, NULL, span_ms, span_ms, -1) != 0) {
        CPE_ERROR(svr->m_em, "%s: set check span: create timer fail!", rank_g_svr_name(svr));
        return -1;
    }

    return 0;
}

int rank_g_svr_record_init(rank_g_svr_t svr, uint32_t record_count, float bucket_ratio) {
    size_t record_buff_capacity;
    set_svr_stub_buff_t record_buff;
    size_t hash_table_buff_capacity;
    set_svr_stub_buff_t hash_table_buff;

    if (svr->m_record_mgr) {
        aom_obj_mgr_free(svr->m_record_mgr);
        svr->m_record_mgr = NULL;
    }

    if (svr->m_record_hash) {
        aom_obj_hash_table_free(svr->m_record_hash);
        svr->m_record_hash = NULL;
    }

    /*初始化记录数组 */
    if (aom_obj_mgr_buf_calc_capacity(&record_buff_capacity, svr->m_record_meta, record_count, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: record init: calc buf capacity fail. record_count=%d!",
            rank_g_svr_name(svr), record_count);
        return -1;
    }

    record_buff = set_svr_stub_buff_check_create(svr->m_stub, "record_buff", record_buff_capacity);
    if (record_buff == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: record init: create record_buff fail, capacity=%d!",
            rank_g_svr_name(svr), (int)record_buff_capacity);
        return -1;
    }

    if (!set_svr_stub_buff_is_init(record_buff)) {
        if (aom_obj_mgr_buf_init(
                svr->m_record_meta,
                set_svr_stub_buff_data(record_buff), set_svr_stub_buff_capacity(record_buff), svr->m_em)
            != 0)
        {
            CPE_ERROR(svr->m_em,  "%s: record init: init record buf fail!", rank_g_svr_name(svr));
            return -1;
        }
    }

    svr->m_record_mgr =aom_obj_mgr_create(svr->m_alloc, set_svr_stub_buff_data(record_buff), set_svr_stub_buff_capacity(record_buff), svr->m_em);
    if (svr->m_record_mgr == NULL) {
        CPE_ERROR(svr->m_em,  "%s: record init: create aom obj mgr fail!", rank_g_svr_name(svr));
        return -1;
    }

    /*初始化记录hash表 */
    hash_table_buff_capacity = aom_obj_hash_table_buf_calc_capacity(svr->m_record_mgr, bucket_ratio);

    hash_table_buff = set_svr_stub_buff_check_create(svr->m_stub, "hash_table_buff", hash_table_buff_capacity);
    if (hash_table_buff == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: record init: create hash_table_buff fail, capacity=%d!",
            rank_g_svr_name(svr), (int)hash_table_buff_capacity);
        return -1;
    }

    if (!set_svr_stub_buff_is_init(hash_table_buff)) {
        if (aom_obj_hash_table_buf_init(
                svr->m_record_mgr, bucket_ratio, 
                dr_meta_key_hash,
                set_svr_stub_buff_data(hash_table_buff), set_svr_stub_buff_capacity(hash_table_buff),
                svr->m_em)
            != 0)
        {
            CPE_ERROR(svr->m_em,  "%s: record init: init hash table buff fail!", rank_g_svr_name(svr));
            return -1;
        }
    }

    svr->m_record_hash =
        aom_obj_hash_table_create(
            svr->m_alloc, svr->m_em,
            svr->m_record_mgr, dr_meta_key_hash, dr_meta_key_cmp,
            set_svr_stub_buff_data(hash_table_buff), set_svr_stub_buff_capacity(hash_table_buff));
    if (svr->m_record_hash == NULL) {
        CPE_ERROR(svr->m_em,  "%s: record init: create aom hash table fail!", rank_g_svr_name(svr));
        return -1;
    }


    CPE_INFO(
        svr->m_em,  "%s: record init: success, record-size=%d, record-buf=%.2fm, hash-buf=%.2fm!",
        rank_g_svr_name(svr), (int)dr_meta_size(aom_obj_mgr_meta(svr->m_record_mgr)),
        record_buff_capacity / 1024.0 / 1024.0, hash_table_buff_capacity / 1024.0 / 1024.0);

    return 0;
}

/* int rank_g_svr_record_init_from_shm(rank_g_svr_t svr, int shm_key) { */
/*     cpe_shm_id_t shmid; */
/*     cpe_shmid_ds shm_info; */
/*     void * data; */

/*     if (svr->m_record_mgr) { */
/*         aom_obj_mgr_free(svr->m_record_mgr); */
/*         svr->m_record_mgr = NULL; */
/*     } */

/*     if (svr->m_record_buf) { */
/*         mem_free(svr->m_alloc, svr->m_record_buf); */
/*         svr->m_record_buf = NULL; */
/*     } */
    
/*     shmid = cpe_shm_get(shm_key); */
/*     if (shmid == -1) { */
/*         CPE_ERROR( */
/*             svr->m_em, "%s: init user data from shm: get shm (key=%d) fail, errno=%d (%s)", */
/*             rank_g_svr_name(svr), shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno())); */
/*         return -1; */
/*     } */

/*     if (cpe_shm_ds_get(shmid, &shm_info) != 0) { */
/*         CPE_ERROR( */
/*             svr->m_em, "%s: init user data from shm: get shm info (key=%d) fail, errno=%d (%s)", */
/*             rank_g_svr_name(svr), shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno())); */
/*         return -1; */
/*     } */

/*     data = cpe_shm_attach(shmid, NULL, 0); */
/*     if (data == NULL) { */
/*         CPE_ERROR( */
/*             svr->m_em, "%s: init user data from shm: attach shm (key=%d, size=%d) fail, errno=%d (%s)", */
/*             rank_g_svr_name(svr), shm_key, shmid, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno())); */
/*         return -1; */
/*     } */

/*     svr->m_record_mgr = aom_obj_mgr_create(svr->m_alloc, data, shm_info.shm_segsz, svr->m_em); */
/*     if (svr->m_record_mgr == NULL) { */
/*         cpe_shm_detach(data); */
/*         CPE_ERROR(svr->m_em, "%s: init user data from shm: create grp obj mgr (from shm) fail!", rank_g_svr_name(svr)); */
/*         return -1; */
/*     } */

/*     if (!dr_meta_compatible(svr->m_record_meta, aom_obj_mgr_meta(svr->m_record_mgr))) { */
/*         cpe_shm_detach(data); */
/*         CPE_ERROR(svr->m_em, "%s: init user data from shm: aom grp meta not compatable!", rank_g_svr_name(svr)); */
/*         return -1; */
/*     } */

/*     return 0; */
/* } */

void rank_g_svr_send_error_response(rank_g_svr_t svr, dp_req_t pkg_head, int16_t error) {
    if (set_pkg_sn(pkg_head)) {
        SVR_RANK_G_RES_ERROR pkg;
        pkg.error = error;

        if (set_svr_stub_send_response_data(
                svr->m_stub,
                set_pkg_from_svr_type(pkg_head), set_pkg_from_svr_id(pkg_head), set_pkg_sn(pkg_head),
                &pkg, sizeof(pkg), svr->m_pkg_meta_res_error,
                NULL, 0)
            != 0)
        {
            CPE_ERROR(svr->m_em, "%s: send error response: send pkg fail!", rank_g_svr_name(svr));
            return;
        }
    }
}
