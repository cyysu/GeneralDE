#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/app/app_library.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_executor_mgr.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "usf/bpg_rsp/bpg_rsp.h"
#include "bpg_rsp_internal_ops.h"

static int bpg_rsp_manage_load_commit_dsp(
    gd_app_context_t app, gd_app_module_t module, bpg_rsp_manage_t bpg_rsp_manage, cfg_t cfg)
{
    cfg_t commit_to;

    if ((commit_to = cfg_find_cfg(cfg, "commit-to"))) {
        bpg_pkg_dsp_t dsp = bpg_pkg_dsp_create(gd_app_alloc(app));
        if (dsp == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create commit dsp fail",
                gd_app_module_name(module));
            return -1;
        }

        if (bpg_pkg_dsp_load(dsp, commit_to, gd_app_em(app)) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create commit dsp: load fail",
                gd_app_module_name(module));
            bpg_pkg_dsp_free(dsp);
            return -1;
        }

        bpg_rsp_manage_set_commit_dsp(bpg_rsp_manage, dsp);
    }

    return 0;
}

static int bpg_rsp_manage_load_queue_infos(
    gd_app_context_t app, gd_app_module_t module, bpg_rsp_manage_t bpg_rsp_manage, cfg_t cfg)
{
    struct cfg_it queue_it;
    cfg_t queue_cfg;

    cfg_it_init(&queue_it, cfg_find_cfg(cfg, "logic-queue"));

    while((queue_cfg = cfg_it_next(&queue_it))) {
        const char * name;
        const char * scope_name;
        bpg_rsp_queue_scope_t scope;

        name = cfg_get_string(queue_cfg, "name", NULL);
        if (name == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create logic queue info: no name!",
                gd_app_module_name(module));
            return -1;
        }

        scope_name = cfg_get_string(queue_cfg, "scope", NULL);
        if (scope_name == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create logic queue %s: scope not configured!",
                gd_app_module_name(module), name);
            return -1;
        }

        if (strcmp(scope_name, "global") == 0) {
            scope = bpg_rsp_queue_scope_global;
        }
        else if (strcmp(scope_name, "client") == 0) {
            scope = bpg_rsp_queue_scope_client;
        }
        else {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create logic queue %s: scope not configured!",
                gd_app_module_name(module), name);
            return -1;
        }

        if (bpg_rsp_queue_info_create(bpg_rsp_manage, name, scope) == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create logic queue %s: create fail!",
                gd_app_module_name(module), name);
            return -1;
        }

        if (cfg_get_int32(queue_cfg, "is-default", 0)) {
            if (bpg_rsp_manage->m_default_queue_info != NULL) {
                CPE_ERROR(
                    gd_app_em(app), "%s: create: create logic queue %s: default queue info already exist, it is %s!",
                    gd_app_module_name(module), name, bpg_rsp_queue_name(bpg_rsp_manage->m_default_queue_info));
                return -1;
            }
        }
    }

    return 0;
}

static int bpg_rsp_manage_load_forward_dsp(
    gd_app_context_t app, gd_app_module_t module, bpg_rsp_manage_t bpg_rsp_manage, cfg_t cfg)
{
    bpg_pkg_dsp_t forward_dsp;

    forward_dsp = bpg_pkg_dsp_create(gd_app_alloc(app));
    if (forward_dsp == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: create forward forward_dsp fail",
            gd_app_module_name(module));
        return -1;
    }

    if (bpg_pkg_dsp_load(forward_dsp, cfg_find_cfg(cfg, "forward-to"), gd_app_em(app)) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: create forward forward_dsp: load fail",
            gd_app_module_name(module));
        bpg_pkg_dsp_free(forward_dsp);
        return -1;
    }
    bpg_rsp_manage_set_forward_dsp(bpg_rsp_manage, forward_dsp);

    return 0;
}

EXPORT_DIRECTIVE
int bpg_rsp_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    bpg_rsp_manage_t bpg_rsp_manage;
    logic_manage_t logic_mgr;
    logic_executor_mgr_t executor_mgr;
    cfg_t child_cfg;
    const char * executor_mgr_name;

    logic_mgr = logic_manage_find_nc(app, cfg_get_string(cfg, "logic-manage", NULL));
    if (logic_mgr == NULL) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: logic-manage %s not exist",
            gd_app_module_name(module), cfg_get_string(cfg, "logic-manage", "default"));
        return -1;
    }

    executor_mgr_name = cfg_get_string(cfg, "executor-manage", NULL);
    if (executor_mgr_name == NULL) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: executor-manage not configured",
            gd_app_module_name(module));
        return -1;
    }

    executor_mgr = logic_executor_mgr_find_nc(app, executor_mgr_name);
    if (executor_mgr == NULL) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: executor-manage %s not exist",
            gd_app_module_name(module), executor_mgr_name);
        return -1;
    }

    bpg_rsp_manage = 
        bpg_rsp_manage_create(
            app,
            gd_app_module_name(module),
            logic_mgr,
            executor_mgr,
            NULL);
    if (bpg_rsp_manage == NULL) {
        return -1;
    }

    if (bpg_rsp_manage_load_commit_dsp(app, module, bpg_rsp_manage, cfg) != 0) {
        bpg_rsp_manage_free(bpg_rsp_manage);
        return -1;
    }

    if (bpg_rsp_manage_load_forward_dsp(app, module, bpg_rsp_manage, cfg) != 0) {
        bpg_rsp_manage_free(bpg_rsp_manage);
        return -1;
    }

    if (bpg_rsp_manage_load_queue_infos(app, module, bpg_rsp_manage, cfg) != 0) {
        bpg_rsp_manage_free(bpg_rsp_manage);
        return -1;
    }

    child_cfg = cfg_find_cfg(cfg, "rsps");
    if (child_cfg) {
        if (bpg_rsp_build(bpg_rsp_manage, child_cfg, gd_app_em(app)) != 0) {
            bpg_rsp_manage_free(bpg_rsp_manage);
            return -1;
        }
    }

    bpg_rsp_manage->m_rsp_max_size =
        cfg_get_uint32(cfg, "rsp-max-size", bpg_rsp_manage->m_rsp_max_size);

    bpg_rsp_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (bpg_rsp_manage->m_debug) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: done. rsp-max-size=%d",
            gd_app_module_name(module), (int)bpg_rsp_manage->m_rsp_max_size);
    }

    return 0;
}

EXPORT_DIRECTIVE
void bpg_rsp_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_rsp_manage_t bpg_rsp_manage;

    bpg_rsp_manage = bpg_rsp_manage_find_nc(app, gd_app_module_name(module));
    if (bpg_rsp_manage) {
        bpg_rsp_manage_free(bpg_rsp_manage);
    }
}
