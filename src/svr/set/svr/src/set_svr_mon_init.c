#include <errno.h>
#include "cpe/utils/file.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/share/set_chanel.h"
#include "svr/set/share/set_repository.h"
#include "set_svr_mon_ops.h"

static int set_svr_mon_app_sync_svr(set_svr_mon_app_t mon_app, set_svr_svr_type_t svr_type, uint16_t svr_id);
static int set_svr_all_root(set_svr_t svr, char * buf, size_t buf_capacity);

int set_svr_app_init_mon(set_svr_mon_t mon) {
    set_svr_t svr = mon->m_svr;
    struct cfg_it mon_app_it;
    cfg_t mon_app_cfg;
    char name_buf[64];
    char all_root[256];
    char set_id[25];

    if (set_svr_all_root(svr, all_root, sizeof(all_root)) != 0) return -1;

    snprintf(name_buf, sizeof(name_buf), "sets.%s", svr->m_set_type);
    snprintf(set_id, sizeof(set_id), "%d", svr->m_set_id);

    cfg_it_init(&mon_app_it, cfg_find_cfg(gd_app_cfg(svr->m_app), name_buf));
    while((mon_app_cfg = cfg_it_next(&mon_app_it))) {
        const char * base = cfg_get_string(mon_app_cfg, "base", NULL);
        const char * bin = cfg_get_string(mon_app_cfg, "bin", NULL);
        const char * svr_type_name = cfg_get_string(mon_app_cfg, "svr-type", NULL);
        struct cfg_it arg_it;
        cfg_t arg_cfg;
        char app_root[256];
        char app_bin[256];
        char pidfile[256];
        set_svr_svr_type_t svr_type;
        set_svr_mon_app_t mon_app;

        if (svr_type_name == NULL) {
            CPE_ERROR(svr->m_em, "%s: load app: svr-type not configured", set_svr_name(svr));
            return -1;
        }

        if (base == NULL) {
            CPE_ERROR(svr->m_em, "%s: load app: base not configured", set_svr_name(svr));
            return -1;
        }

        if (bin == NULL) {
            CPE_ERROR(svr->m_em, "%s: load bin: bin not configured", set_svr_name(svr));
            return -1;
        }

        svr_type = set_svr_svr_type_find_by_name(svr, svr_type_name);
        if (svr_type == NULL) {
            CPE_ERROR(svr->m_em, "%s: load bin: svr-type %s not exist", set_svr_name(svr), svr_type_name);
            return -1;
        }

        snprintf(app_root, sizeof(app_root), "%s/%s", all_root, base);
        if (!dir_exist(app_root, svr->m_em)) {
            CPE_ERROR(svr->m_em, "%s: load app: root dir %s not exist", set_svr_name(svr), app_root);
            return -1;
        }

        snprintf(app_bin, sizeof(app_bin), "%s/%s", app_root, bin);
        if (access(app_bin, X_OK) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: load app: bin %s check execute fail, error=%d (%s)",
                set_svr_name(svr), app_bin, errno, strerror(errno));
            return -1;
        }

        snprintf(pidfile, sizeof(pidfile), "%s/%s.pid", svr->m_repository_root, base);

        mon_app = set_svr_mon_app_create(mon, svr_type, app_bin, pidfile);
        if (mon_app == NULL) return -1;

        if (set_svr_mon_app_add_arg(mon_app, app_bin) != 0) {
            CPE_ERROR(svr->m_em, "%s: load app: add ap_bin arg fail", set_svr_name(svr));
            return -1;
        }

        cfg_it_init(&arg_it, cfg_find_cfg(mon_app_cfg, "args"));
        while((arg_cfg = cfg_it_next(&arg_it))) {
            const char * arg_value = cfg_as_string(arg_cfg, NULL);

            if (set_svr_mon_app_add_arg(mon_app, arg_value) != 0) {
                CPE_ERROR(
                    svr->m_em, "%s: load app: bin %s add arg %s fail",
                    set_svr_name(svr), app_bin, arg_value);
                return -1;
            }
        }

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

        CPE_INFO(svr->m_em, "%s: load app: %s", set_svr_name(svr), app_bin);
    }

    return 0;
}

int set_svr_all_root(set_svr_t svr, char * buf, size_t buf_capacity) {
    const char * app_root = gd_app_root(svr->m_app);
    const char * dir_end = NULL;
    const char * p;
    int len;

    if (app_root == NULL) {
        CPE_ERROR(svr->m_em, "%s: get project root fail!", set_svr_name(svr));
        return -1;
    }

    for(p = strchr(app_root, '/'); p; p = strchr(p + 1, '/')) {
        dir_end = p;
    }

    if (dir_end == NULL) {
        CPE_ERROR(svr->m_em, "%s: get project root fail, gd_app_root=%s", set_svr_name(svr), app_root);
        return -1;
    }

    len = dir_end - app_root;
    if (len > buf_capacity) {
        CPE_ERROR(svr->m_em, "%s: get project root overflow!, buf_capacity=%d", set_svr_name(svr), (int)buf_capacity);
        return -1;
    }

    memcpy(buf, app_root, len);
    buf[len] = 0;

    return 0;
}

static int set_svr_mon_app_sync_svr(set_svr_mon_app_t mon_app, set_svr_svr_type_t svr_type, uint16_t svr_id) {
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;
    set_svr_svr_t svr_svr;
    int shmid;
    cfg_t svr_cfg;
    const char * str_rq_size;
    uint64_t rq_size;
    const char * str_wq_size;
    uint64_t wq_size;

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

    if ((str_rq_size = cfg_get_string(svr_cfg, "rq-size", NULL)) == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: rq-size not configured!",
            set_svr_name(svr), svr_type->m_svr_type_name, svr_id);
        return -1;
    }

    if (cpe_str_parse_byte_size(&rq_size, str_rq_size) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: rq-size %s format error!",
            set_svr_name(svr), svr_type->m_svr_type_name, svr_id, str_rq_size);
        return -1;
    }

    if ((str_wq_size = cfg_get_string(svr_cfg, "wq-size", NULL)) == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: wq-size not configured!",
            set_svr_name(svr), svr_type->m_svr_type_name, svr_id);
        return -1;
    }

    if (cpe_str_parse_byte_size(&wq_size, str_wq_size) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: wq-size %s format error!",
            set_svr_name(svr), svr_type->m_svr_type_name, svr_id, str_wq_size);
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

    svr_svr->m_chanel = set_chanel_shm_init(shmid, wq_size, rq_size, svr->m_em);
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
