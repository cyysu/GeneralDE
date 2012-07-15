#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_types.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/pom_mgr/pom_mgr_manage.h"
#include "pom_mgr_internal_ops.h"

/* int pom_manage_init_from_memory(pom_manage_t pom_manage, cfg_t cfg) { */
/*     return 0; */
/* } */

EXPORT_DIRECTIVE
int pom_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    pom_manage_t pom_manage;
    cfg_t load_cfg;
    LPDRMETA meta;

    pom_manage =
        pom_manage_create(
            app, gd_app_module_name(module), gd_app_alloc(app), gd_app_em(app));
    if (pom_manage == NULL) return -1;

    pom_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    
    //load_cfg = cfg_find_cfg(cfg, "load-from-memory", 

    return 0;
}

EXPORT_DIRECTIVE
void pom_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    pom_manage_t pom_manage;

    pom_manage = pom_manage_find_nc(app, gd_app_module_name(module));
    if (pom_manage) {
        pom_manage_free(pom_manage);
    }
}

