#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/otm/otm_manage.h"
#include "otm_internal_ops.h"

static void otm_manage_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_otm_manage = {
    "gd_otm_manage",
    otm_manage_clear
};

otm_manage_t
otm_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    otm_manage_t mgr;
    nm_node_t mgr_node;

    assert(name);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct otm_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (otm_manage_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_em = em;
    mgr->m_debug = 0;

    if (cpe_hash_table_init(
            &mgr->m_timers,
            alloc,
            (cpe_hash_fun_t) otm_timer_hash,
            (cpe_hash_cmp_t) otm_timer_cmp,
            CPE_HASH_OBJ2ENTRY(otm_timer, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "otm_manage_create: init timer hash table fail!");
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_otm_manage);

    return mgr;
}

static void otm_manage_clear(nm_node_t node) {
    otm_manage_t mgr;
    mgr = (otm_manage_t)nm_node_data(node);

    otm_timer_free_all(mgr);

    cpe_hash_table_fini(&mgr->m_timers);
}

void otm_manage_free(otm_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_otm_manage) return;
    nm_node_free(mgr_node);
}

otm_manage_t otm_manage_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_otm_manage) return NULL;
    return (otm_manage_t)nm_node_data(node);
}

otm_manage_t otm_manage_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_otm_manage) return NULL;
    return (otm_manage_t)nm_node_data(node);
}

gd_app_context_t otm_manage_app(otm_manage_t mgr) {
    return mgr->m_app;
}

const char * otm_manage_name(otm_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
otm_manage_name_hs(otm_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}


EXPORT_DIRECTIVE
int otm_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    otm_manage_t otm_manage;

    otm_manage =
        otm_manage_create(
            app,
            gd_app_module_name(module),
            gd_app_alloc(app),
            gd_app_em(app));
    if (otm_manage == NULL) return -1;

    otm_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (otm_manage->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            otm_manage_name(otm_manage));
    }

    return 0;
}

EXPORT_DIRECTIVE
void otm_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    otm_manage_t otm_manage;

    otm_manage = otm_manage_find_nc(app, gd_app_module_name(module));
    if (otm_manage) {
        otm_manage_free(otm_manage);
    }
}

