#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/file.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/app/app_log.h"
#include "set_svr_ops.h"
#include "set_svr_mon_ops.h"
#include "set_svr_center_ops.h"

static int set_svr_app_init_calc_repository(
    gd_app_context_t app, const char * set_type, uint16_t set_id,
    error_monitor_t em, char * buf, size_t buf_capacity);

static int set_svr_app_init_load_svr_types(set_svr_t svr) {
    struct cfg_it svr_it;
    cfg_t svr_cfg;

    cfg_it_init(&svr_it, cfg_find_cfg(gd_app_cfg(svr->m_app), "svr_types"));
    while((svr_cfg = cfg_it_next(&svr_it))) {
        set_svr_svr_type_t set_svr_type;

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

static int set_svr_app_init_center(struct set_svr_center * center, cfg_t cfg, const char * center_addr) {
    const char * str_read_block_size;
    const char * str_max_pkg_size;

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

    if (center_addr) {
        char ip[64];
        short port;
        const char * sep;

        sep = strchr(center_addr, ':');
        if (sep) {
            int len = sep - center_addr;

            if ((len + 1)  >= sizeof(ip)) {
                CPE_ERROR(center->m_svr->m_em, "%s: create: center: center_addr %s too long!", set_svr_name(center->m_svr), center_addr);
                return -1;
            }

            memcpy(ip, center_addr, len);
            ip[len] = 0;
            port = atoi(sep + 1);
        }
        else {
            int len = strlen(center_addr);

            memcpy(ip, center_addr, len);
            ip[len] = 0;
            port = 8099;
        }

        if (set_svr_center_set_svr(center, ip, port) != 0) {
            CPE_ERROR(
                center->m_svr->m_em, "%s: create: center: set svr %s:%d fail!",
                set_svr_name(center->m_svr), ip, (int)port);
            return -1;
        }
    }
    else {
        if (center->m_svr->m_debug) {
            CPE_INFO(center->m_svr->m_em, "%s: create: center: no center ip!", set_svr_name(center->m_svr));
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
int set_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_t set_svr;
    const char * set_type;
    const char * set_id_str;
    uint16_t set_id;
    const char * value;
    const char * str_ringbuf_size;
    uint64_t ringbuf_size;
    char repository_root_buf[128];
    const char * center_addr;

    center_addr = gd_app_arg_find(app, "--center");

    if ((set_type = gd_app_arg_find(app, "--set-type")) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: --set-type not configured in command!", gd_app_module_name(module));
        return -1;
    }

    if ((set_id_str = gd_app_arg_find(app, "--set-id")) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: --set-id not configured in command!", gd_app_module_name(module));
        return -1;
    }
    set_id = atoi(set_id_str);

    value = gd_app_arg_find(app, "--repository");
    if (value) {
        snprintf(repository_root_buf, sizeof(repository_root_buf), "%s/%s-%d", value, set_type, set_id);
    }
    else {
        if (set_svr_app_init_calc_repository(app, set_type, set_id, gd_app_em(app), repository_root_buf, sizeof(repository_root_buf)) != 0) {
            APP_CTX_ERROR(app, "%s: create: --repository get fail!", gd_app_module_name(module));
            return -1;
        }
    }

    if (dir_mk_recursion(repository_root_buf, DIR_DEFAULT_MODE, gd_app_em(app), gd_app_alloc(app)) != 0) {
        APP_CTX_ERROR(app, "%s: create: repository_root %s create fail!", gd_app_module_name(module), repository_root_buf);
        return -1;
    }
    
    if ((str_ringbuf_size = cfg_get_string(cfg, "ringbuf-size", NULL)) == NULL) {
        APP_CTX_ERROR(app, "%s: create: ringbuf-size not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cpe_str_parse_byte_size(&ringbuf_size, str_ringbuf_size) != 0) {
        APP_CTX_ERROR(
            app, "%s: create: read ringbuf-size %s fail!",
            gd_app_module_name(module), str_ringbuf_size);
        return -1;
    }

    set_svr =
        set_svr_create(
            app, gd_app_module_name(module), repository_root_buf, set_type, set_id,
            gd_app_alloc(app), gd_app_em(app));
    if (set_svr == NULL) return -1;

    set_svr->m_debug = cfg_get_int8(cfg, "debug", set_svr->m_debug);

    if (set_svr_set_ringbuf_size(set_svr, ringbuf_size) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set ringbuf-size %d fail!", gd_app_module_name(module), (int)ringbuf_size);
        set_svr_free(set_svr);
        return -1;
    }

    if (set_svr_app_init_center(set_svr->m_center, cfg_find_cfg(cfg, "center"), center_addr) != 0) {
        set_svr_free(set_svr);
        return -1;
    }

    if (set_svr_app_init_load_svr_types(set_svr) != 0) {
        set_svr_free(set_svr);
        return -1;
    }

    set_svr->m_mon->m_restart_wait_ms = (tl_time_span_t)cfg_get_uint32(cfg, "mon.restart-wait-ms", 1000);
    if (set_svr_app_init_mon(set_svr->m_mon) != 0) {
        set_svr_free(set_svr);
        return -1;
    }

    if (center_addr) {
        set_svr_center_apply_evt(set_svr->m_center, set_svr_center_fsm_evt_start);
    }

    if (set_svr->m_debug) {
        CPE_INFO(
            set_svr->m_em, "%s: creat: success, repository_root=%s, set-tpe=%s, set-id=%d",
            set_svr_name(set_svr), repository_root_buf, set_type, set_id);
    }

    set_svr_mon_app_start_all(set_svr->m_mon);

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

int set_svr_app_init_calc_repository(
    gd_app_context_t app, const char * set_type, uint16_t set_id,
    error_monitor_t em, char * buf, size_t buf_capacity)
{
    struct cpe_str_buf str_buf = CPE_STR_BUF_INIT(buf, buf_capacity);
    const char * app_root = gd_app_root(app);
    const char * name_begin = NULL;
    const char * name_end = NULL;
    const char * home;
    size_t home_len;

    if (app_root) {
        const char * p;
        for(p = strchr(app_root, '/'); p; p = strchr(p + 1, '/')) {
            name_begin = name_end;
            name_end = p + 1;
        }
    }

    if (name_begin == NULL || name_end == NULL) {
        CPE_ERROR(em, "set_repository_root: root %s format error!", app_root);
        return -1;
    }

    home = getenv("HOME");
    home_len = strlen(home);

    cpe_str_buf_append(&str_buf, home, home_len);
    if (home[home_len - 1] != '/') {
        cpe_str_buf_cat(&str_buf, "/");
    }
    cpe_str_buf_cat(&str_buf, ".");
    cpe_str_buf_append(&str_buf, name_begin, name_end - name_begin - 1);
    cpe_str_buf_cat_printf(&str_buf, "/%s-%d", set_type, set_id);

    return cpe_str_buf_is_overflow(&str_buf) ? -1 : 0;
}

