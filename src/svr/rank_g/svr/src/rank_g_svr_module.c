#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "rank_g_svr_ops.h"

static int rank_g_svr_load_def(rank_g_svr_t svr, cfg_t cfg);
static int rank_g_svr_load_record(rank_g_svr_t svr);

EXPORT_DIRECTIVE
int rank_g_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    rank_g_svr_t rank_g_svr;
    const char * request_recv_at;
    cfg_t rank_def;

    if ((request_recv_at = cfg_get_string(cfg, "request-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: rank_g-request-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    rank_g_svr =
        rank_g_svr_create(
            app, gd_app_module_name(module), stub,
            gd_app_alloc(app), gd_app_em(app));
    if (rank_g_svr == NULL) return -1;

    rank_g_svr->m_debug = cfg_get_int8(cfg, "debug", rank_g_svr->m_debug);

    if (rank_g_svr_set_request_recv_at(rank_g_svr, request_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set request-recv-at %s fail!", gd_app_module_name(module), request_recv_at);
        rank_g_svr_free(rank_g_svr);
        return -1;
    }

    rank_def = cfg_find_cfg(gd_app_cfg(app), "rank-def");
    if (rank_def == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: rank-def not configured.", gd_app_module_name(module));
        rank_g_svr_free(rank_g_svr);
        return -1;
    }

    if (rank_g_svr_load_def(rank_g_svr, rank_def) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load rank-def fail.", gd_app_module_name(module));
        rank_g_svr_free(rank_g_svr);
        return -1;
    }

    if (rank_g_svr_load_record(rank_g_svr) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: init record fail.", gd_app_module_name(module));
        rank_g_svr_free(rank_g_svr);
        return -1;
    }

    if (rank_g_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void rank_g_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    rank_g_svr_t rank_g_svr;

    rank_g_svr = rank_g_svr_find_nc(app, gd_app_module_name(module));
    if (rank_g_svr) {
        rank_g_svr_free(rank_g_svr);
    }
}

static LPDRMETA rank_g_svr_load_meta(rank_g_svr_t svr, const char * str_meta) {
    dr_store_manage_t store_mgr;
    dr_store_t store;
    char const * sep;
    char lib_name[64];
    LPDRMETA meta;

    store_mgr = dr_store_manage_find_nc(svr->m_app, NULL);
    if (store_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: store_mgr not exist!", rank_g_svr_name(svr));
        return NULL;
    }

    sep = strchr(str_meta, '.');
    if (sep == NULL || (sep - str_meta) > (sizeof(lib_name) - 1)) {
        CPE_ERROR(svr->m_em, "%s: pkg-meta %s format error or overflow!", rank_g_svr_name(svr), str_meta);
        return NULL;
    }
    memcpy(lib_name, str_meta, sep - str_meta);
    lib_name[sep - str_meta] = 0;

    store = dr_store_find(store_mgr, lib_name);
    if (store == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: metalib %s not exist in %s!",
            rank_g_svr_name(svr), lib_name, dr_store_manage_name(store_mgr));
        return NULL;
    }

    meta = dr_lib_find_meta_by_name(dr_store_lib(store), sep + 1);
    if (meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: metalib %s have no meta %s!",
            rank_g_svr_name(svr), lib_name, sep + 1);
        return NULL;
    }

    return meta;
}

static int rank_g_svr_load_def(rank_g_svr_t svr, cfg_t cfg) {
    const char * str_meta;
    const char * str_uid_entry;
    struct cfg_it indexes_it;
    cfg_t index_cfg;

    str_meta = cfg_get_string(cfg, "meta", NULL);
    if (str_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: rank-def.meta not configured.", rank_g_svr_name(svr));
        return -1;
    }

    str_uid_entry = cfg_get_string(cfg, "user-id", NULL);
    if (str_uid_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: rank-def.user-id not configured.", rank_g_svr_name(svr));
        return -1;
    }

    svr->m_record_meta = rank_g_svr_load_meta(svr, str_meta);
    if (svr->m_record_meta == NULL) return -1;
    svr->m_record_size = dr_meta_size(svr->m_record_meta);

    cfg_it_init(&indexes_it, cfg_find_cfg(cfg, "indexes"));
    while((index_cfg = cfg_it_next(&indexes_it))) {
        rank_g_svr_index_t index;
        uint16_t id;
        const char * attr;

        if (cfg_try_get_uint16(index_cfg, "id", &id) != 0) {
            CPE_ERROR(svr->m_em, "%s: create: create index info: id not configured", rank_g_svr_name(svr));
            return -1;
        }

        attr = cfg_get_string(index_cfg, "attr", NULL);
        if (attr == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: create index info %d: attr not configured", rank_g_svr_name(svr), id);
            return -1;
        }

        index = rank_g_svr_index_create(svr, id, attr);
        if (index == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: create index info %d fail", rank_g_svr_name(svr), id);
            return -1;
        }
    }

    return 0;
}

static int rank_g_svr_load_record(rank_g_svr_t svr) {
    const char * str_record_count;
    uint32_t record_count;
    const char * str_bucket_ratio;
    float bucket_ratio = 1.5;

    str_record_count = gd_app_arg_find(svr->m_app, "--record-count");
    if (str_record_count == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: record init: record-count not conrigured", rank_g_svr_name(svr));
        return -1;
    }

    record_count = strtol(str_record_count, NULL, 10);

    if ((str_bucket_ratio = gd_app_arg_find(svr->m_app, "--bucket-ratio"))) {
        bucket_ratio = strtof(str_bucket_ratio, NULL);
    }

    if (rank_g_svr_record_init(svr, record_count, bucket_ratio) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: record init: init fail, record_count=%d", rank_g_svr_name(svr), record_count);
        return -1;
    }

    return 0;
}
