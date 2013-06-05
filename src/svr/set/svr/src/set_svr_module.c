#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "set_svr_ops.h"
#include "set_svr_center_ops.h"

static int set_svr_app_init_load_svr_types(set_svr_t svr) {
    set_svr_svr_type_t set_svr_type;
    struct cfg_it svr_it;
    cfg_t svr_cfg;

    cfg_it_init(&svr_it, cfg_find_cfg(gd_app_cfg(svr->m_app), "svr_types"));
    while((svr_cfg = cfg_it_next(&svr_it))) {
        set_svr_type = set_svr_svr_type_create(svr, cfg_name(svr_cfg));
        if (set_svr_type == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load_svr_types: check_create %s fail!",
                set_svr_name(svr), cfg_name(svr_cfg));
            return -1;
        }
        else {
            if (svr->m_debug) {
                CPE_INFO(
                    svr->m_em, "%s: create: load_svr_types: create svr type %s(%d)!",
                    set_svr_name(svr), set_svr_type->m_svr_type_name, set_svr_type->m_svr_type_id);
            }
        }
    }

    return 0;
}

static int set_svr_app_init_center(struct set_svr_center * center, cfg_t cfg) {
    const char * ip;
    short port;
    const char * str_read_block_size;
    const char * str_max_pkg_size;

    ip = cfg_get_string(cfg, "ip", NULL);
    port = cfg_get_int16(cfg, "port", 0);

    center->m_reconnect_span_ms = cfg_get_uint32(cfg, "reconnect-span-ms", center->m_reconnect_span_ms);
    center->m_update_span_s = cfg_get_uint32(cfg, "update-span-s", center->m_update_span_s);

    if ((str_read_block_size = cfg_get_string(cfg, "read-block-size", NULL))) {
        uint64_t read_block_size;
        if (cpe_str_parse_byte_size(&read_block_size, str_read_block_size) != 0) {
            CPE_ERROR(
                center->m_svr->m_em, "%s: create: center: read read-block-size %s fail!",
                set_svr_name(center->m_svr), str_read_block_size);
            return -1;
        }

        center->m_read_block_size = (uint32_t)read_block_size;
    }

    if ((str_max_pkg_size = cfg_get_string(cfg, "max-pkg-size", NULL))) {
        uint64_t max_pkg_size;
        if (cpe_str_parse_byte_size(&max_pkg_size, str_max_pkg_size) != 0) {
            CPE_ERROR(
                center->m_svr->m_em, "%s: create: center: read max-pkg-size %s fail!",
                set_svr_name(center->m_svr), str_max_pkg_size);
            return -1;
        }

        center->m_max_pkg_size = (uint32_t)max_pkg_size;
    }

    if (set_svr_center_set_svr(center, ip, port) != 0) {
        CPE_ERROR(
            center->m_svr->m_em, "%s: create: center: set svr %s:%d fail!",
            set_svr_name(center->m_svr), ip, (int)port);
        return -1;
    }

    return 0;
}


EXPORT_DIRECTIVE
int set_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_t set_svr;
    /* const char * ip; */
    /* short port; */
    const char * str_ringbuf_size;
    uint64_t ringbuf_size;
    uint32_t local_search_span_s;

    /* ip = cfg_get_string(cfg, "ip", ""); */
    /* port = cfg_get_int16(cfg, "port", 0); */

    if ((str_ringbuf_size = cfg_get_string(cfg, "ringbuf-size", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: ringbuf-size not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cpe_str_parse_byte_size(&ringbuf_size, str_ringbuf_size) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: read ringbuf-size %s fail!",
            gd_app_module_name(module), str_ringbuf_size);
        return -1;
    }

    if (cfg_try_get_uint32(cfg, "local-search-span-s", &local_search_span_s) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: local-search-span-s not configured!", gd_app_module_name(module));
        return -1;
    }

    set_svr =
        set_svr_create(
            app, gd_app_module_name(module),
            gd_app_alloc(app), gd_app_em(app));
    if (set_svr == NULL) return -1;

    set_svr->m_debug = cfg_get_int8(cfg, "debug", set_svr->m_debug);

    if (set_svr_set_ringbuf_size(set_svr, ringbuf_size) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set ringbuf-size %d fail!", gd_app_module_name(module), (int)ringbuf_size);
        set_svr_free(set_svr);
        return -1;
    }

    if (set_svr_app_init_center(set_svr->m_center, cfg_find_cfg(cfg, "center")) != 0) {
        set_svr_free(set_svr);
        return -1;
    }

    if (set_svr_app_init_load_svr_types(set_svr) != 0) {
        set_svr_free(set_svr);
        return -1;
    }

    set_svr_start_local_search_timer(set_svr, local_search_span_s * 1000);

    set_svr_center_apply_evt(set_svr->m_center, set_svr_center_fsm_evt_start);

    return 0;
}

EXPORT_DIRECTIVE
void set_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    set_svr_t set_svr;

    set_svr = set_svr_find_nc(app, gd_app_module_name(module));
    if (set_svr) {
        set_svr_free(set_svr);
    }
}
