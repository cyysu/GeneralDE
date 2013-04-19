#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_listener.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "center_svr_ops.h"

extern char g_metalib_svr_center_pro[];
static void center_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_center_svr = {
    "svr_center_svr",
    center_svr_clear
};

center_svr_t
center_svr_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct center_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct center_svr));
    if (svr_node == NULL) return NULL;

    svr = (center_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_debug = 0;
    svr->m_cvt = NULL;
    svr->m_listener = NULL;
    svr->m_read_chanel_size = 4 * 1024;
    svr->m_write_chanel_size = 1024;
    svr->m_conn_timeout_ms = 500 * 1000;
    svr->m_listener = NULL;
    svr->m_client_data_mgr = NULL;
    svr->m_process_count_per_tick = 10;
    svr->m_max_pkg_size = 1024 * 1024 * 5;
    TAILQ_INIT(&svr->m_conns);

    svr->m_record_meta =
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_center_pro, "svr_center_cli_record");
    if (svr->m_record_meta == NULL) {
        CPE_ERROR(em, "%s: create find record meta fail!", name);
        nm_node_free(svr_node);
        return NULL;
    }

    svr->m_pkg_meta =
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_center_pro, "svr_center_pkg");
    if (svr->m_pkg_meta == NULL) {
        CPE_ERROR(em, "%s: create find pkg meta fail!", name);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_groups,
            alloc,
            (cpe_hash_fun_t) center_cli_group_hash,
            (cpe_hash_cmp_t) center_cli_group_eq,
            CPE_HASH_OBJ2ENTRY(center_cli_group, m_hh),
            -1) != 0)
    {
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_datas,
            alloc,
            (cpe_hash_fun_t) center_cli_data_hash,
            (cpe_hash_cmp_t) center_cli_data_eq,
            CPE_HASH_OBJ2ENTRY(center_cli_data, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_groups);
        nm_node_free(svr_node);
        return NULL;
    }

    mem_buffer_init(&svr->m_mem_data_buf, svr->m_alloc);
    mem_buffer_init(&svr->m_outgoing_encode_buf, svr->m_alloc);
    mem_buffer_init(&svr->m_incoming_pkg_buf, svr->m_alloc);
    mem_buffer_init(&svr->m_outgoing_pkg_buf, svr->m_alloc);
    mem_buffer_init(&svr->m_dump_buffer, svr->m_alloc);

    nm_node_set_type(svr_node, &s_nm_node_type_center_svr);

    return svr;
}

static void center_svr_clear(nm_node_t node) {
    center_svr_t svr;
    svr = (center_svr_t)nm_node_data(node);

    if (svr->m_cvt) {
        dr_cvt_free(svr->m_cvt);
        svr->m_cvt = NULL;
    }

    mem_buffer_clear(&svr->m_outgoing_encode_buf);
    mem_buffer_clear(&svr->m_incoming_pkg_buf);
    mem_buffer_clear(&svr->m_outgoing_pkg_buf);
    mem_buffer_clear(&svr->m_dump_buffer);

    /*清理服务监听接口 */
    if (svr->m_listener) {
        net_listener_free(svr->m_listener);
        svr->m_listener = NULL;
    }

    /*清理连接 */
    center_cli_conn_free_all(svr);

    /*清理客户端数据缓存 */
    center_cli_data_free_all(svr);
    assert(cpe_hash_table_count(&svr->m_groups) == 0);
    assert(cpe_hash_table_count(&svr->m_datas) == 0);
    cpe_hash_table_fini(&svr->m_groups);
    cpe_hash_table_fini(&svr->m_datas);
    if (svr->m_client_data_mgr) {
        aom_obj_mgr_free(svr->m_client_data_mgr);
        svr->m_client_data_mgr = NULL;
    }
    mem_buffer_clear(&svr->m_mem_data_buf);
}

int center_svr_set_cvt(center_svr_t svr, const char * cvt_name) {
    if (svr->m_cvt) dr_cvt_free(svr->m_cvt);

    svr->m_cvt = dr_cvt_create(svr->m_app, cvt_name);
    if (svr->m_cvt == NULL) {
        CPE_ERROR(svr->m_em, "%s: set cvt %s fail!", center_svr_name(svr), cvt_name);
        return -1;
    } 

    return 0;
}

int center_svr_set_listener(center_svr_t svr, const char * ip, short port, int acceptQueueSize) {
    if (svr->m_listener) {
        net_listener_free(svr->m_listener);
    }

    svr->m_listener =
        net_listener_create(
            gd_app_net_mgr(svr->m_app),
            center_svr_name(svr),
            ip,
            port,
            acceptQueueSize,
            center_svr_accept,
            svr);
    if (svr->m_listener == NULL) {
        return -1;
    }

    return 0;
}

static void center_svr_build_datas_from_aom(center_svr_t svr) {
    struct aom_obj_it it;
    SVR_CENTER_CLI_RECORD * record;

    aom_objs(svr->m_client_data_mgr, &it);

    while((record = aom_obj_it_next(&it))) {
        center_cli_data_create(svr, record);
    }
}

int center_svr_init_clients_from_shm(center_svr_t svr, int shm_key) {
    cpe_shm_id_t shmid;
    cpe_shmid_ds shm_info;
    void * data;

    if (svr->m_client_data_mgr) {
        aom_obj_mgr_free(svr->m_client_data_mgr);
        svr->m_client_data_mgr = NULL;
    }

    shmid = cpe_shm_get(shm_key);
    if (shmid == -1) {
        CPE_ERROR(svr->m_em, "%s: init from shm %d: shm_get fail!", center_svr_name(svr), shm_key);
        return -1;
    }

    if (cpe_shm_ds_get(shmid, &shm_info) != 0) {
        CPE_ERROR(svr->m_em, "%s: init from shm %d: shm_ds_get fail!", center_svr_name(svr), shm_key);
        return -1;
    }

    data = cpe_shm_attach(shmid, NULL, 0);
    if (data == NULL) {
        CPE_ERROR(svr->m_em, "%s: init from shm %d: shm_attach fail!", center_svr_name(svr), shm_key);
        return -1;
    }

    svr->m_client_data_mgr = aom_obj_mgr_create(svr->m_alloc, data, shm_info.shm_segsz, svr->m_em);
    if (svr->m_client_data_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: init from shm %d: create aom_obj_mgr fail!", center_svr_name(svr), shm_key);
        cpe_shm_detach(data);
        return -1;
    }

    if (!dr_meta_compatible(svr->m_record_meta, aom_obj_mgr_meta(svr->m_client_data_mgr))) {
        aom_obj_mgr_free(svr->m_client_data_mgr);
        svr->m_client_data_mgr = NULL;
        cpe_shm_detach(data);
        CPE_ERROR(svr->m_em, "%s: init from shm %d: aom grp meta not compatable!", center_svr_name(svr), shm_key);
        return -1;
    }

    center_svr_build_datas_from_aom(svr);

    return 0;
}

int center_svr_init_clients_from_mem(center_svr_t svr, size_t capacity) {
    void * buf;

    if (svr->m_client_data_mgr) {
        center_cli_data_free_all(svr);
        assert(cpe_hash_table_count(&svr->m_groups) == 0);
        assert(cpe_hash_table_count(&svr->m_datas) == 0);
        aom_obj_mgr_free(svr->m_client_data_mgr);
        svr->m_client_data_mgr = NULL;
    }

    if (mem_buffer_set_size(&svr->m_mem_data_buf, capacity) != 0
        || (buf = mem_buffer_make_continuous(&svr->m_mem_data_buf, 0)) == NULL)
    {
        CPE_ERROR(svr->m_em, "%s: init from mem: alloc buff fail, capacity=%d!", center_svr_name(svr), (int)capacity);
        return -1;
    }

    if (aom_obj_mgr_buf_init(svr->m_record_meta, buf, capacity, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: init from mem: aom_obj_mgr_buf_init fail!", center_svr_name(svr));
        return -1;
    }

    svr->m_client_data_mgr = aom_obj_mgr_create(svr->m_alloc, buf, capacity, svr->m_em);
    if (svr->m_client_data_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: init from mem: create aom_obj_mgr fail!", center_svr_name(svr));
        return -1;
    }

    center_svr_build_datas_from_aom(svr);

    return 0;
}

gd_app_context_t center_svr_app(center_svr_t svr) {
    return svr->m_app;
}

void center_svr_free(center_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_center_svr) return;
    nm_node_free(svr_node);
}

center_svr_t
center_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_center_svr) return NULL;
    return (center_svr_t)nm_node_data(node);
}

center_svr_t
center_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_center_svr) return NULL;
    return (center_svr_t)nm_node_data(node);
}

const char * center_svr_name(center_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
center_svr_name_hs(center_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

void * center_svr_get_incoming_pkg_buff(center_svr_t svr, size_t capacity) {
    if (mem_buffer_size(&svr->m_incoming_pkg_buf) < capacity) {
        if (mem_buffer_set_size(&svr->m_incoming_pkg_buf, capacity) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: create pkg buf for data size %d fail",
                center_svr_name(svr), (int)capacity);
            return NULL;
        }
    }

    return mem_buffer_make_continuous(&svr->m_incoming_pkg_buf, 0);
}

SVR_CENTER_PKG *
center_svr_get_res_pkg_buff(center_svr_t svr, SVR_CENTER_PKG * req, size_t capacity) {
    SVR_CENTER_PKG * res;

    if (mem_buffer_size(&svr->m_outgoing_pkg_buf) < capacity) {
        if (mem_buffer_set_size(&svr->m_outgoing_pkg_buf, capacity) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: create pkg buf for data size %d fail",
                center_svr_name(svr), (int)capacity);
            return NULL;
        }
    }

    res = mem_buffer_make_continuous(&svr->m_outgoing_pkg_buf, 0);
    bzero(res, capacity);

    if (req) {
        res->cmd = req->cmd + 1;
        res->sn = req->sn;
    }

    return res;
}

