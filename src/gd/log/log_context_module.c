#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "log_internal_ops.h"

EXPORT_DIRECTIVE
int log_context_app_init(gd_app_context_t context, gd_app_module_t module, cfg_t cfg) {
    struct log_context * ctx;
    struct cfg_it child_it;
    cfg_t child_cfg;
    error_monitor_t em;
    
    assert(context);
    em = gd_app_em(context);

    ctx = log_context_create(context, em, gd_app_alloc(context));
    if (ctx == NULL) return -1;

    ctx->m_debug = cfg_get_int32(cfg, "debug", 0);

    cfg_it_init(&child_it, cfg_find_cfg(cfg, "categories"));

    while((child_cfg = cfg_it_next(&child_it))) {
        const char * em_name;

        child_cfg = cfg_child_only(child_cfg);
        if (child_cfg == NULL) {
            CPE_ERROR(em, "%s: create log em: format error!", gd_app_module_name(module));
            continue;
        }

        em_name  = cfg_name(child_cfg);
        if (log4c_em_create(ctx, em_name, child_cfg) != 0) {
            CPE_ERROR(em, "%s: create log em %s: fail!", gd_app_module_name(module), em_name);
            continue;
        }
        else {
            if (ctx->m_debug) {
                CPE_INFO(em, "%s: create log em %s: success!", gd_app_module_name(module), em_name);
            }
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
void log_context_app_fini(gd_app_context_t context) {
    struct log_context * ctx;
    ctx = log_context_find(context);

    if (ctx) {
        log_context_free(ctx);
    }
}

EXPORT_DIRECTIVE
int log_context_global_init(void) {
    return log4c_init() == 0 ? 0 : -1;
}

EXPORT_DIRECTIVE
void log_context_global_fini() {
    log4c_fini();
}
