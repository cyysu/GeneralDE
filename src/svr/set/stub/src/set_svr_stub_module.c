#include <assert.h>
#include <signal.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/service.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/share/set_repository.h"
#include "set_svr_stub_internal_ops.h"

static LPDRMETA set_svr_stub_load_pkg_meta(
    dr_store_manage_t store_mgr, error_monitor_t em, const char * module,
    const char * svr_type_name, const char * str_pkg_meta);
static int set_svr_stub_load_connect_svrs(set_svr_stub_t stub, dr_store_manage_t store_mgr, cfg_t svr_types_cfg);
static set_svr_svr_info_t
set_svr_stub_load_svr_info(set_svr_stub_t stub, dr_store_manage_t store_mgr, cfg_t svr_types_cfg, const char * svr_type_name);

EXPORT_DIRECTIVE
int set_svr_stub_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t svr;
    const char * request_dispatch_to;
    const char * outgoing_recv_at;
    const char * value;
    const char * svr_type_name;
    const char * str_svr_id;
    uint16_t svr_id;
    struct cfg_it child_it;
    cfg_t child_cfg;
    const char * pidfile;
    int shmid;
    set_chanel_t chanel;
    cfg_t svr_types_cfg;
    dr_store_manage_t store_mgr;

    svr_types_cfg = cfg_find_cfg(gd_app_cfg(app), "svr_types");

    store_mgr = dr_store_manage_find(app, NULL);
    if (store_mgr == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: default store_mgr not exist!", gd_app_module_name(module));
        return -1;
    }

    pidfile = gd_app_arg_find(app, "--pidfile");
    if (pidfile == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: pidfile not configured in command line!", gd_app_module_name(module));
        return -1;
    }

    gd_stop_on_signal(SIGUSR1);

    shmid = cpe_shm_key_gen(pidfile, 'a');
    if (shmid == -1) {
        CPE_ERROR(gd_app_em(app), "%s: create: gen shm key fail, error=%d (%s)!", gd_app_module_name(module), errno, strerror(errno));
        return -1;
    }

    request_dispatch_to = cfg_get_string(cfg, "request-send-to", NULL);
    if (request_dispatch_to == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: request-send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    outgoing_recv_at = cfg_get_string(cfg, "outgoing-recv-at", NULL);
    if (outgoing_recv_at == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: outgoing-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    svr_type_name = cfg_get_string(cfg, "svr-type", NULL);
    if (svr_type_name == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: svr-type not configured!", gd_app_module_name(module));
        return -1;
    }

    str_svr_id = gd_app_arg_find(app, "--app-id");
    if (str_svr_id == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: app-id not configured in command line!", gd_app_module_name(module));
        return -1;
    }

    svr_id = atoi(str_svr_id);

    svr = set_svr_stub_create(app, gd_app_module_name(module), svr_id, gd_app_alloc(app), gd_app_em(app));
    if (svr == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: create fail!", gd_app_module_name(module));
        return -1;
    }
    svr->m_debug = cfg_get_int32(cfg, "debug", 0);

    svr->m_svr_type = set_svr_stub_load_svr_info(svr, store_mgr, svr_types_cfg, svr_type_name);
    if (svr->m_svr_type == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: svr type %s not exist!", gd_app_module_name(module), svr_type_name);
        set_svr_stub_free(svr);
        return -1;
    }

    if (set_svr_stub_load_connect_svrs(svr, store_mgr, svr_types_cfg) != 0) {
        set_svr_stub_free(svr);
        return -1;
    }

    if (set_svr_stub_write_pidfile(svr, pidfile) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: is already runing!", gd_app_module_name(module));
        set_svr_stub_free(svr);
        return -1;
    }

    chanel = set_chanel_shm_attach(shmid, gd_app_em(app));
    if (chanel == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: open chanel fail!", gd_app_module_name(module));
        set_svr_stub_free(svr);
        return -1;
    }
    set_svr_stub_set_chanel(svr, chanel);

    if (set_svr_stub_set_request_dispatch_to(svr, request_dispatch_to) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set request-send-to %s fail!",
            gd_app_module_name(module), request_dispatch_to);
        set_svr_stub_free(svr);
        return -1;
    }

    if ((value = cfg_get_string(cfg, "response-send-to", NULL))) {
        if (set_svr_stub_set_response_dispatch_to(svr, value) != 0) {
            CPE_ERROR(gd_app_em(app), "%s: create: set response-send-to %s fail!", gd_app_module_name(module), value);
            set_svr_stub_free(svr);
            return -1;
        }
    }

    cfg_it_init(&child_it, cfg_find_cfg(cfg, "svr-dispatch-infos"));
    while((child_cfg = cfg_it_next(&child_it))) {
        const char * from_svr_type_name;
        set_svr_svr_info_t from_svr_info;

        from_svr_type_name = cfg_get_string(child_cfg, "svr-type", NULL);
        if (from_svr_type_name == NULL) {
            CPE_ERROR(gd_app_em(app), "%s: create: read svr-dispatch-infos: svr-type not configured!", gd_app_module_name(module));
            set_svr_stub_free(svr);
            return -1;
        }

        from_svr_info = set_svr_svr_info_find_by_name(svr, from_svr_type_name);
        if (from_svr_info == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read svr-dispatch-infos: svr-type %s unknown!",
                gd_app_module_name(module), from_svr_type_name);
            set_svr_stub_free(svr);
            return -1;
        }

        if ((value = cfg_get_string(child_cfg, "notify-send-to", NULL))) {
            if (from_svr_info->m_notify_dispatch_to) {
                mem_free(svr->m_alloc, from_svr_info->m_notify_dispatch_to);
            }
            from_svr_info->m_notify_dispatch_to = cpe_hs_create(svr->m_alloc, value);
        }

        if ((value = cfg_get_string(child_cfg, "response-send-to", NULL))) {
            if (from_svr_info->m_response_dispatch_to) {
                mem_free(svr->m_alloc, from_svr_info->m_response_dispatch_to);
            }
            from_svr_info->m_response_dispatch_to = cpe_hs_create(svr->m_alloc, value);
        }
    }

    if (set_svr_stub_set_outgoing_recv_at(svr, outgoing_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set outgoing-recv-at %s fail!",
            gd_app_module_name(module), outgoing_recv_at);
        set_svr_stub_free(svr);
        return -1;
    }

    if (svr->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done, svr-type=%s, svr-id=%d",
            gd_app_module_name(module), svr_type_name, svr_id);
    }

    return 0;
}

EXPORT_DIRECTIVE
void set_svr_stub_app_fini(gd_app_context_t app, gd_app_module_t module) {
    set_svr_stub_t set_svr_stub;

    set_svr_stub = set_svr_stub_find_nc(app, gd_app_module_name(module));
    if (set_svr_stub) {
        set_svr_stub_free(set_svr_stub);
    }
}

static LPDRMETA set_svr_stub_load_pkg_meta(
    dr_store_manage_t store_mgr, error_monitor_t em, const char * module,
    const char * svr_type_name, const char * str_pkg_meta)
{
    dr_store_t store;
    char const * sep;
    char lib_name[64];
    LPDRMETA pkg_meta;

    sep = strchr(str_pkg_meta, '.');
    if (sep == NULL || (sep - str_pkg_meta) > (sizeof(lib_name) - 1)) {
        CPE_ERROR(em, "%s: svr-type %s: pkg-meta %s format error or overflow!", module, svr_type_name, str_pkg_meta);
        return NULL;
    }
    memcpy(lib_name, str_pkg_meta, sep - str_pkg_meta);
    lib_name[sep - str_pkg_meta] = 0;

    store = dr_store_find(store_mgr, lib_name);
    if (store == NULL) {
        CPE_ERROR(
            em, "%s: svr-type %s: metalib %s not exist in %s!",
            module, svr_type_name, lib_name, dr_store_manage_name(store_mgr));
        return NULL;
    }

    pkg_meta = dr_lib_find_meta_by_name(dr_store_lib(store), sep + 1);
    if (pkg_meta == NULL) {
        CPE_ERROR(
            em, "%s: svr-type %s: metalib %s have no meta %s!",
            module, svr_type_name, svr_type_name, sep + 1);
        return NULL;
    }

    return pkg_meta;
}

static set_svr_svr_info_t set_svr_stub_load_svr_info(set_svr_stub_t stub, dr_store_manage_t store_mgr, cfg_t svr_types_cfg, const char * svr_type_name) {
    uint16_t svr_type_id;
    set_svr_svr_info_t svr_info;
    cfg_t svr_cfg;
    const char * str_pkg_meta;
    const char * svr_pkg_data_entry;

    svr_cfg = cfg_find_cfg(svr_types_cfg, svr_type_name);
    if (svr_cfg == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: %s: load svr cfg fail!",
            set_svr_stub_name(stub), svr_type_name);
        return NULL;
    }

    if (cfg_try_get_uint16(svr_cfg, "id", &svr_type_id) != 0) {
        CPE_ERROR(
            stub->m_em, "%s: %s: id not configured!",
            set_svr_stub_name(stub), svr_type_name);
        return NULL;
    }

    svr_info = set_svr_svr_info_create(stub, svr_type_name, svr_type_id);
    if (svr_info == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: %s: create svr_info fail!",
            set_svr_stub_name(stub), svr_type_name);
        return NULL;
    }

    str_pkg_meta = cfg_get_string(svr_cfg, "pkg-meta", NULL);
    if (str_pkg_meta == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: %s: pkg-meta not configured!",
            set_svr_stub_name(stub), svr_type_name);
        return NULL;
    }

    svr_info->m_pkg_meta = set_svr_stub_load_pkg_meta(store_mgr, stub->m_em, set_svr_stub_name(stub), svr_type_name, str_pkg_meta);
    if (svr_info->m_pkg_meta == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: %s: load pkg-meta %s fail!",
            set_svr_stub_name(stub), svr_type_name, str_pkg_meta);
        return NULL;
    }

    svr_pkg_data_entry = cfg_get_string(svr_cfg, "pkg-meta-data", NULL);
    if (svr_pkg_data_entry) {
        LPDRMETAENTRY pkg_data_entry;
        LPDRMETA pkg_data_meta;
        LPDRMETAENTRY pkg_cmd_entry;
        int i;

        pkg_data_entry = dr_meta_find_entry_by_name(svr_info->m_pkg_meta, svr_pkg_data_entry);
        if (pkg_data_entry == NULL) {
            CPE_ERROR(
                stub->m_em, "%s: %s: pkg-meta %s no data entry %s!",
                set_svr_stub_name(stub), svr_type_name, str_pkg_meta, svr_pkg_data_entry);
            return NULL;
        }

        pkg_data_meta = dr_entry_ref_meta(pkg_data_entry);
        if (pkg_data_meta == NULL || dr_meta_type(pkg_data_meta) != CPE_DR_TYPE_UNION) {
            CPE_ERROR(
                stub->m_em, "%s: %s: data_entry %s.%s not union!",
                set_svr_stub_name(stub), svr_type_name, str_pkg_meta, svr_pkg_data_entry);
            return NULL;
        }

        pkg_cmd_entry = dr_entry_select_entry(pkg_data_entry);
        if (pkg_cmd_entry == NULL) {
            CPE_ERROR(
                stub->m_em, "%s: %s: pkg-meta %s data entry %s no select entry!",
                set_svr_stub_name(stub), svr_type_name, str_pkg_meta, svr_pkg_data_entry);
            return NULL;
        }

        svr_info->m_pkg_cmd_entry = pkg_cmd_entry;
        svr_info->m_pkg_data_entry = pkg_data_entry;

        for(i = 0; i < dr_meta_entry_num(pkg_data_meta); ++i) {
            LPDRMETAENTRY cmd_entry = dr_meta_entry_at(pkg_data_meta, i);
            if (set_svr_cmd_info_create(stub, svr_info, cmd_entry) == NULL) {
                CPE_ERROR(
                    stub->m_em, "%s: %s: add cmd %s fail!",
                    set_svr_stub_name(stub), svr_type_name, dr_entry_name(cmd_entry));
                return NULL;
            }
        }
    }

    return svr_info;
}

static int set_svr_stub_load_connect_svrs(set_svr_stub_t stub, dr_store_manage_t store_mgr, cfg_t svr_types_cfg) {
    cfg_t svr_cfg;
    struct cfg_it cfg_it;
    cfg_t connect_to_cfg;
    const char * svr_type_name;

    svr_type_name = stub->m_svr_type->m_svr_type_name;

    svr_cfg = cfg_find_cfg(svr_types_cfg, svr_type_name);
    if (svr_cfg == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: create type of svr type %s: no config!",
            set_svr_stub_name(stub), svr_type_name);
        return -1;
    }

    cfg_it_init(&cfg_it, cfg_find_cfg(svr_cfg, "connect-to"));
    while((connect_to_cfg = cfg_it_next(&cfg_it))) {

        const char * to_svr_type_name;

        to_svr_type_name = cfg_as_string(connect_to_cfg, NULL);
        if (to_svr_type_name == NULL) {
            CPE_ERROR(
                stub->m_em, "%s: %s -> ???: connect-to config error!",
                set_svr_stub_name(stub), svr_type_name);
            return -1;
        }

        if (set_svr_stub_load_svr_info(stub, store_mgr, svr_types_cfg, to_svr_type_name) == NULL) {
            return -1;
        }
    }

    return 0;
}
