#include <assert.h>
#include <errno.h>
#include "cpe/utils/file.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/share/set_chanel.h"
#include "svr/set/share/set_repository.h"
#include "set_svr_mon_ops.h"

static int set_svr_all_root(const char * app_bin, char * buf, size_t buf_capacity, error_monitor_t em);
static int set_svr_mon_app_sync_svr(set_svr_mon_app_t mon_app, set_svr_svr_type_t svr_type, uint16_t svr_id);
static int set_svr_app_init_load_local_env(set_svr_mon_t mon, cfg_t g_env);
static int set_svr_app_init_create_app(set_svr_mon_t mon, const char * all_root, cfg_t g_env, const char * app_name, cfg_t app_args);

int set_svr_app_init_mon(set_svr_mon_t mon) {
    set_svr_t svr = mon->m_svr;
    struct cfg_it mon_app_it;
    cfg_t mon_app_cfg;
    char name_buf[64];
    char all_root[256];
    cfg_t cfg_set_root;
    cfg_t cfg_tmp;
    cfg_t g_env = NULL;

    if (set_svr_all_root(gd_app_argv(svr->m_app)[0], all_root, sizeof(all_root), svr->m_em) != 0) goto INIT_MON_FAIL;

    /*读入set相关的配置 */
    snprintf(name_buf, sizeof(name_buf), "sets.%s", svr->m_set_type);
    cfg_set_root = cfg_find_cfg(gd_app_cfg(svr->m_app), name_buf);

    /*读入全局的参数配置 */
    g_env = cfg_create(svr->m_alloc);
    if (g_env == NULL) {
        CPE_ERROR(svr->m_em, "%s: load apps: create local env cfg fail", set_svr_name(svr));
        goto INIT_MON_FAIL;
    }

    /*      从set的配置中读取 */
    if ((cfg_tmp = cfg_find_cfg(cfg_set_root, "env"))) {
        if (cfg_merge(g_env, cfg_tmp, cfg_merge_use_new, svr->m_em) != 0) {
            CPE_ERROR(svr->m_em, "%s: load apps: merge set env fail", set_svr_name(svr));
            goto INIT_MON_FAIL;
        }
    }

    /*      从本机的运行环境中读取 */
    if (set_svr_app_init_load_local_env(mon, g_env) != 0) {
        goto INIT_MON_FAIL;
    }

    /*遍历应用配置，加载app */
    cfg_it_init(&mon_app_it, cfg_find_cfg(cfg_set_root, "apps"));
    while((mon_app_cfg = cfg_it_next(&mon_app_it))) {
        cfg_t app_args;
        const char * app_name;

        if (cfg_type(mon_app_cfg) == CPE_CFG_TYPE_STRING) {
            app_name = cfg_as_string(mon_app_cfg, NULL);
            assert(app_name);
            app_args = NULL;
        }
        else if (cfg_type(mon_app_cfg) == CPE_CFG_TYPE_STRUCT) {
            mon_app_cfg = cfg_child_only(mon_app_cfg);
            if (mon_app_cfg == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: load apps: app cfg type error, struct have not only child!",
                    set_svr_name(svr));
                goto INIT_MON_FAIL;
            }

            app_name = cfg_name(mon_app_cfg);
            assert(app_name);
            app_args = mon_app_cfg;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: load apps: app cfg type error, type=%d!",
                set_svr_name(svr), cfg_type(mon_app_cfg));
            goto INIT_MON_FAIL;
        }

        if (set_svr_app_init_create_app(mon, all_root, g_env, app_name, app_args) != 0) {
            goto INIT_MON_FAIL;
        }
    }

    return 0;

INIT_MON_FAIL:
    if (g_env) cfg_free(g_env);
    return -1;
}

static int set_svr_mon_app_sync_svr(set_svr_mon_app_t mon_app, set_svr_svr_type_t svr_type, uint16_t svr_id) {
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;
    set_svr_svr_t svr_svr;
    int shmid;
    cfg_t svr_cfg;

    svr_svr = set_svr_svr_find(svr, svr_type->m_svr_type_id, svr_id);
    if (svr_svr) {
        if (svr_svr->m_category == set_svr_svr_local) {
            if (svr->m_debug >= 2) {
                CPE_INFO(
                    svr->m_em, "%s: on find svr %s.%d: already exist, ignore!",
                    set_svr_name(svr), svr_type->m_svr_type_name, svr_id);
            }
        }
        else {
            CPE_INFO(
                svr->m_em, "%s: on find svr %s.%d: already exist, update to local!",
                set_svr_name(svr), svr_type->m_svr_type_name, svr_id);

            set_svr_svr_set_category(svr_svr, set_svr_svr_local);
        }

        return 0;
    }

    svr_cfg = cfg_find_cfg(gd_app_cfg(svr->m_app), "svr_types");
    svr_cfg = cfg_find_cfg(svr_cfg, svr_type->m_svr_type_name);
    if (svr_cfg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: config of svr_type not exist!",
            set_svr_name(svr), svr_type->m_svr_type_name, svr_id);
        return -1;
    }

    shmid = cpe_shm_key_gen(mon_app->m_pidfile, 'a');
    if (shmid == -1) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: get shmid at %s fail!",
            set_svr_name(svr), svr_type->m_svr_type_name, svr_id, mon_app->m_pidfile);
        return -1;
    }

    svr_svr = set_svr_svr_create(svr, svr_type, svr_id, set_svr_svr_local);
    if (svr_svr == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: create fail!",
            set_svr_name(svr), svr_type->m_svr_type_name, svr_id);
        return -1;
    }

    svr_svr->m_chanel = set_chanel_shm_init(shmid, mon_app->m_wq_size, mon_app->m_rq_size, svr->m_em);
    if (svr_svr->m_chanel == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: attach chanel fail!",
            set_svr_name(svr), svr_type->m_svr_type_name, svr_id);
        set_svr_svr_free(svr_svr);
        return -1;
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: on find svr %s.%d: found new svr!",
            set_svr_name(svr), svr_type->m_svr_type_name, svr_id);
    }

    return 0;
}

static int set_svr_all_root(const char * app_bin, char * buf, size_t buf_capacity, error_monitor_t em) {
    const char * dir_end_1 = NULL;
    const char * dir_end_2 = NULL;
    const char * dir_end_3 = NULL;
    const char * p;
    int len;

    if (app_bin == NULL) {
        CPE_ERROR(em, "set_svr: get project root fail!");
        return -1;
    }

    for(p = strchr(app_bin, '/'); p; p = strchr(p + 1, '/')) {
        dir_end_1 = dir_end_2;
        dir_end_2 = dir_end_3;
        dir_end_3 = p;
    }

    if (dir_end_1 == NULL) {
        CPE_ERROR(em, "set_svr: get project root fail, gd_app_root=%s", app_bin);
        return -1;
    }

    len = dir_end_1 - app_bin;
    if (len > buf_capacity) {
        CPE_ERROR(em, "set_svr: get project root overflow!, buf_capacity=%d", (int)buf_capacity);
        return -1;
    }

    memcpy(buf, app_bin, len);
    buf[len] = 0;

    return 0;
}

static const char * set_svr_app_read_value(set_svr_t svr, cfg_t g_env, cfg_t app_env, const char * input, char * buf, size_t buf_capacity) {
    const char * value;

    if (input == NULL) return input;

    if (input[0] != '$') return input;

    if (app_env && (value = cfg_get_string_cvt(app_env, input + 1, NULL, buf, buf_capacity))) return value;

    if (g_env && (value = cfg_get_string_cvt(g_env, input + 1, NULL, buf, buf_capacity))) return value;

    return NULL;
}

static int set_svr_app_init_create_app(set_svr_mon_t mon, const char * all_root, cfg_t g_env, const char * app_name, cfg_t app_args) {
    set_svr_t svr = mon->m_svr;
    const char * base;
    const char * bin;
    const char * svr_type_name;
    struct cfg_it arg_it;
    const char * str_rq_size;
    uint64_t rq_size;
    const char * str_wq_size;
    uint64_t wq_size;
    cfg_t arg_cfg;
    char app_root[256];
    char app_bin[256];
    char pidfile[256];
    set_svr_svr_type_t svr_type;
    set_svr_mon_app_t mon_app;
    cfg_t app_cfg;
    cfg_t app_env;
    char buf[128];
    char set_id[25];

    snprintf(buf, sizeof(buf), "apps.%s", app_name);
    app_cfg = cfg_find_cfg(gd_app_cfg(svr->m_app), buf);
    if (app_cfg == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: app not configured, path=%s", set_svr_name(svr), app_name, buf);
        return -1;
    }
    app_env = cfg_find_cfg(app_args, "env");

    base = set_svr_app_read_value(svr, g_env, app_env, cfg_get_string(app_cfg, "base", NULL), NULL, 0);
    if (base == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: base not configured", set_svr_name(svr), app_name);
        return -1;
    }

    bin = set_svr_app_read_value(svr, g_env, app_env, cfg_get_string(app_cfg, "bin", NULL), NULL, 0);
    if (bin == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: bin not configured", set_svr_name(svr), app_name);
        return -1;
    }

    svr_type_name = cfg_get_string(app_cfg, "svr-type", NULL);
    if (svr_type_name == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: svr-type not configured", set_svr_name(svr), app_name);
        return -1;
    }

    svr_type = set_svr_svr_type_find_by_name(svr, svr_type_name);
    if (svr_type == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: svr-type %s not exist", set_svr_name(svr), app_name, svr_type_name);
        return -1;
    }

    snprintf(app_root, sizeof(app_root), "%s/%s", all_root, base);
    if (!dir_exist(app_root, svr->m_em)) {
        CPE_ERROR(svr->m_em, "%s: load app %s: root dir %s not exist", set_svr_name(svr), app_name, app_root);
        return -1;
    }

    snprintf(app_bin, sizeof(app_bin), "%s/%s", app_root, bin);
    if (access(app_bin, X_OK) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: load app %s: check %s execute fail, error=%d (%s)",
            set_svr_name(svr), app_name, app_bin, errno, strerror(errno));
        return -1;
    }

    snprintf(pidfile, sizeof(pidfile), "%s/%s.pid", svr->m_repository_root, base);

    if ((str_rq_size = set_svr_app_read_value(svr, g_env, app_env, cfg_get_string(app_cfg, "rq-size", NULL), buf, sizeof(buf))) == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: rq-size not configured!", set_svr_name(svr), app_name);
        return -1;
    }

    if (cpe_str_parse_byte_size(&rq_size, str_rq_size) != 0) {
        CPE_ERROR(svr->m_em, "%s: load app %s: rq-size %s format error!", set_svr_name(svr), app_name, str_rq_size);
        return -1;
    }

    if ((str_wq_size = set_svr_app_read_value(svr, g_env, app_env, cfg_get_string(app_cfg, "wq-size", NULL), buf, sizeof(buf))) == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: wq-size not configured!", set_svr_name(svr), app_name);
        return -1;
    }

    if (cpe_str_parse_byte_size(&wq_size, str_wq_size) != 0) {
        CPE_ERROR(svr->m_em, "%s: load app %s: wq-size %s format error!", set_svr_name(svr), app_name, str_wq_size);
        return -1;
    }

    mon_app = set_svr_mon_app_create(mon, svr_type, app_bin, pidfile, rq_size, wq_size);
    if (mon_app == NULL) return -1;

    if (set_svr_mon_app_add_arg(mon_app, app_bin) != 0) {
        CPE_ERROR(svr->m_em, "%s: load app: add ap_bin arg fail", set_svr_name(svr));
        return -1;
    }

    cfg_it_init(&arg_it, cfg_find_cfg(app_cfg, "args"));
    while((arg_cfg = cfg_it_next(&arg_it))) {
        const char * arg_name;
        const char * arg_value;

        switch(cfg_type(arg_cfg)) {
        case CPE_CFG_TYPE_STRING:
            arg_name = cfg_as_string(arg_cfg, NULL);
            assert(arg_name);
            arg_value = NULL;
            break;
        case CPE_CFG_TYPE_STRUCT: {
            const char * orig_arg_value;
            char value_buf[128];

            arg_cfg = cfg_child_only(arg_cfg);
            if (arg_cfg == NULL) {
                CPE_ERROR(svr->m_em, "%s: load app %s: arg format error!", set_svr_name(svr), app_name);
                return -1;
            }

            arg_name = cfg_name(arg_cfg);
            assert(arg_name);

            orig_arg_value = cfg_as_string_cvt(arg_cfg, NULL, value_buf, sizeof(value_buf));
            if (orig_arg_value == NULL) {
                CPE_ERROR(svr->m_em, "%s: load app %s: arg %s value is not string!", set_svr_name(svr), app_name, arg_name);
                return -1;
            }

            arg_value = set_svr_app_read_value(svr, g_env, app_env, orig_arg_value, buf, sizeof(buf));
            if (arg_value == NULL) {
                CPE_ERROR(svr->m_em, "%s: load app %s: arg %s read value %s fail!", set_svr_name(svr), app_name, arg_name, orig_arg_value);
                return -1;
            }

            break;
        }
        default:
            CPE_ERROR(svr->m_em, "%s: load app %s: arg type error, type=%d!", set_svr_name(svr), app_name, cfg_type(arg_cfg));
            return -1;
        }

        if (set_svr_mon_app_add_arg(mon_app, arg_name) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: load app %s: add arg name %s fail",
                set_svr_name(svr), app_name, arg_name);
            return -1;
        }

        if (arg_value && set_svr_mon_app_add_arg(mon_app, arg_value) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: load app %s: add arg value %s fail",
                set_svr_name(svr), app_name, arg_value);
            return -1;
        }
    }

    snprintf(set_id, sizeof(set_id), "%d", svr->m_set_id);

    if (set_svr_mon_app_add_arg(mon_app, "--pidfile") != 0
        || set_svr_mon_app_add_arg(mon_app, pidfile) != 0
        || set_svr_mon_app_add_arg(mon_app, "--root") != 0
        || set_svr_mon_app_add_arg(mon_app, app_root) != 0
        || set_svr_mon_app_add_arg(mon_app, "--app-id") != 0
        || set_svr_mon_app_add_arg(mon_app, set_id) != 0
        ) 
    {
        CPE_ERROR(svr->m_em, "%s: load app: add common arg fail", set_svr_name(svr));
        return -1;
    }

    if (set_svr_mon_app_sync_svr(mon_app, mon_app->m_svr_type, svr->m_set_id) != 0) return -1;

    CPE_INFO(svr->m_em, "%s: load app %s: success", set_svr_name(svr), app_name);

    return 0;
}

static int set_svr_app_init_load_local_env(set_svr_mon_t mon, cfg_t g_env) {
    set_svr_t svr = mon->m_svr;
    char path[128];

    snprintf(path, sizeof(path), "%s/env.yml", svr->m_repository_root);

    if (!file_exist(path, svr->m_em)) {
        CPE_INFO(svr->m_em, "%s: load env from %s: skip!", set_svr_name(svr), path);
        return 0;
    }

    if (cfg_read_file(g_env, path, cfg_replace, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: load env from %s: load cfg fail", set_svr_name(svr), path);
        return -1;
    }

    CPE_INFO(svr->m_em, "%s: load env from %s: success!", set_svr_name(svr), path);
    return 0;
}

