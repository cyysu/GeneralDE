#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/app/app_library.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "usf/pom_gs/pom_gs_agent.h"
#include "pom_gs_internal_ops.h"

EXPORT_DIRECTIVE
int pom_gs_agent_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    void * pom_grp_meta_bin;
    dr_store_manage_t store_mgr;
    pom_grp_meta_t pom_grp_meta;
    pom_gs_agent_t pom_gs_agent;
    dr_store_t store;
    const char * metalib_name;
    const char * pom_grp_meta_symbol_name;
    const char * main_entry;
    const char * key;

    store_mgr = dr_store_manage_default(app);
    if (store_mgr == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s create: get default dr_sotre_manage fail!",
            gd_app_module_name(module));
        return -1;
    }

    main_entry = cfg_get_string(cfg, "main-entry", NULL);
    if (main_entry == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: main-entry not configured!",
            gd_app_module_name(module));
        return -1;
    }

    key = cfg_get_string(cfg, "key", NULL);
    if (key == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: key not configured!",
            gd_app_module_name(module));
        return -1;
    }

    metalib_name = cfg_get_string(cfg, "data-meta-lib", NULL);
    if (metalib_name == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: data-meta-lib not configured!",
            gd_app_module_name(module));
        return -1;
    }

    store = dr_store_find(store_mgr, metalib_name);
    if (store == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s create: metalib %s not exist!",
            gd_app_module_name(module), metalib_name);
        return -1;
    }

    pom_grp_meta_symbol_name = cfg_get_string(cfg, "pom-grp-meta-symbol", NULL);
    if (pom_grp_meta_symbol_name == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: pom-grp-meta-symbol not configured!",
            gd_app_module_name(module));
        return -1;
    }

    pom_grp_meta_bin = gd_app_lib_sym(NULL, pom_grp_meta_symbol_name, gd_app_em(app));
    if (pom_grp_meta_bin == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s:  find pom-grp-meta-symbol %s: error!",
            gd_app_module_name(module), pom_grp_meta_symbol_name);
        return -1;
    }

    pom_grp_meta = 
        pom_grp_meta_build_from_bin(
            gd_app_alloc(app),
            pom_grp_meta_bin,
            dr_store_lib(store),
            gd_app_em(app));
    if (pom_grp_meta == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: create grp meta fail!",
            gd_app_module_name(module));
        return -1;
    }

    pom_gs_agent =
        pom_gs_agent_create(
            app, gd_app_module_name(module), 
            pom_grp_meta,
            main_entry,
            key,
            gd_app_alloc(app), gd_app_em(app));
    if (pom_gs_agent == NULL) return -1;

    pom_gs_agent->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (pom_gs_agent->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            pom_gs_agent_name(pom_gs_agent));
    }

    return 0;
}

EXPORT_DIRECTIVE
void pom_gs_agent_app_fini(gd_app_context_t app, gd_app_module_t module) {
    pom_gs_agent_t pom_gs_agent;

    pom_gs_agent = pom_gs_agent_find_nc(app, gd_app_module_name(module));
    if (pom_gs_agent) {
        pom_gs_agent_free(pom_gs_agent);
    }
}

