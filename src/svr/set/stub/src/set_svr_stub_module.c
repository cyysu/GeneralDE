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
#include "svr/center/agent/center_agent.h"
#include "svr/center/agent/center_agent_svr_type.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/share/set_repository.h"
#include "set_svr_stub_internal_ops.h"

EXPORT_DIRECTIVE
int set_svr_stub_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    center_agent_t center_agent;
    set_svr_stub_t svr;
    const char * request_dispatch_to;
    const char * outgoing_recv_at;
    const char * svr_type_name;
    const char * value;
    const char * str_svr_id;
    uint16_t svr_id;
    center_agent_svr_type_t svr_type;
    struct cfg_it child_it;
    cfg_t child_cfg;
    const char * pidfile;
    int shmid;
    set_chanel_t chanel;

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

    center_agent = center_agent_find_nc(app, cfg_get_string(cfg, "center-agent", "center_agent"));
    if (center_agent == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: find center-agent %s fail.",
            gd_app_module_name(module), cfg_get_string(cfg, "center-agent", "center_agent"));
        return -1;
    }

    svr_type = center_agent_svr_type_check_create(center_agent, svr_type_name);
    if (svr_type == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: svr_type %s not exist!", gd_app_module_name(module), svr_type_name);
        return -1;
    }

    svr = set_svr_stub_create(app, gd_app_module_name(module), center_agent, svr_type, svr_id, gd_app_alloc(app), gd_app_em(app));
    if (svr == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: create fail!", gd_app_module_name(module));
        return -1;
    }
    svr->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (set_svr_stub_write_pidfile(svr, pidfile) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: is already runing!", gd_app_module_name(module));
        return -1;
    }

    chanel = set_chanel_shm_attach(shmid, gd_app_em(app));
    if (chanel == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: open chanel fail!", gd_app_module_name(module));
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
        center_agent_svr_type_t from_svr_type;

        from_svr_type_name = cfg_get_string(child_cfg, "svr-type", NULL);
        if (from_svr_type_name == NULL) {
            CPE_ERROR(gd_app_em(app), "%s: create: read svr-dispatch-infos: svr-type not configured!", gd_app_module_name(module));
            set_svr_stub_free(svr);
            return -1;
        }

        from_svr_type = center_agent_svr_type_lsearch_by_name(center_agent, from_svr_type_name);
        if (from_svr_type == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read svr-dispatch-infos: svr-type %s unknown!",
                gd_app_module_name(module), from_svr_type_name);
            set_svr_stub_free(svr);
            return -1;
        }
        
        if ((value = cfg_get_string(child_cfg, "notify-send-to", NULL))) {
            if (set_svr_stub_set_svr_notify_dispatch_to(svr, center_agent_svr_type_id(from_svr_type), value) != 0) {
                CPE_ERROR(
                    gd_app_em(app), "%s: create: set %s notify-send-to %s fail!",
                    gd_app_module_name(module), from_svr_type_name, value);
                set_svr_stub_free(svr);
                return -1;
            }
        }

        if ((value = cfg_get_string(child_cfg, "response-send-to", NULL))) {
            if (set_svr_stub_set_svr_response_dispatch_to(svr, center_agent_svr_type_id(from_svr_type), value) != 0) {
                CPE_ERROR(
                    gd_app_em(app), "%s: create: set %s response-send-to %s fail!",
                    gd_app_module_name(module), from_svr_type_name, value);
                set_svr_stub_free(svr);
                return -1;
            }
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

