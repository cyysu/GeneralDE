#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/mongo_use/id_generator.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_sp.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "account_svr_ops.h"

extern char g_metalib_svr_account_pro[];

EXPORT_DIRECTIVE
int account_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    set_logic_sp_t set_sp;
    account_svr_t account_svr;
    set_logic_rsp_manage_t rsp_manage;
    mongo_cli_proxy_t db;
    mongo_id_generator_t id_generator;

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    set_sp = set_logic_sp_find_nc(app, cfg_get_string(cfg, "set-sp", NULL));
    if (set_sp == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-sp %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-sp", "default"));
        return -1;
    }

    rsp_manage = set_logic_rsp_manage_find_nc(app, cfg_get_string(cfg, "rsp-manage", NULL));
    if (rsp_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: rsp-manage %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "rsp-manage", "default"));
        return -1;
    }

    db = mongo_cli_proxy_find_nc(app, cfg_get_string(cfg, "db", NULL));
    if (db == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: db %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "db", "default"));
        return -1;
    }

    id_generator = mongo_id_generator_find_nc(app, "id-generator");
    if (id_generator == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: add: get id-generate fail!", gd_app_module_name(module));
        return -1;
    }

    account_svr =
        account_svr_create(
            app, gd_app_module_name(module),
            stub, set_sp, rsp_manage, db, id_generator,
            gd_app_alloc(app), gd_app_em(app));
    if (account_svr == NULL) return -1;

    if (set_logic_rsp_build(
            account_svr->m_rsp_manage,
            cfg_find_cfg(gd_app_cfg(app), "rsps"), (LPDRMETALIB)g_metalib_svr_account_pro, gd_app_em(app)) != 0)
    {
        CPE_ERROR(gd_app_em(app), "%s: create: load rsps fail!", gd_app_module_name(module));
        account_svr_free(account_svr);
        return -1;
    }

    account_svr->m_debug = cfg_get_int8(cfg, "debug", account_svr->m_debug);

    if (account_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void account_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    account_svr_t account_svr;

    account_svr = account_svr_find_nc(app, gd_app_module_name(module));
    if (account_svr) {
        account_svr_free(account_svr);
    }
}

