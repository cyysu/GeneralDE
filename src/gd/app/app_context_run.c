#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_manage.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "gd/app/app_tl.h"
#include "app_internal_ops.h"

static int gd_app_chdir_to_root(gd_app_context_t context) {
    if (context->m_root) {
        if (chdir(context->m_root) != 0) {
            CPE_ERROR(gd_app_em(context), "change root to %s fail!", context->m_root);
            return -1;
        }
    }

    return 0;
}

static int gd_app_run_i(gd_app_context_t context) {
    int rv;

    assert(context);

    if (gd_app_load_childs(context) != 0) {
        CPE_ERROR(context->m_em, "load child apps fail!");
        return -1;
    }

    if (!gd_app_flag_is_enable(context, gd_app_flag_no_auto_load)) {
        if (gd_app_cfg_reload(context) != 0) {
            return -1;
        }

        if (gd_app_modules_load(context) != 0) {
            return -1;
        }
    }

    if (context->m_main == NULL) {
        CPE_ERROR(context->m_em, "no main function to runing!");
        gd_app_modules_unload(context);
        return -1;
    }

    rv = context->m_main(context, context->m_fun_ctx);

    if (!gd_app_flag_is_enable(context, gd_app_flag_delay_module_unload)) {
        gd_app_modules_unload(context);
        gd_app_tick_chain_free(context);
    }

    return rv;
}

int gd_app_start_inline(gd_app_context_t context) {
    if (context->m_state != gd_app_init) {
        APP_CTX_ERROR(context, "gd_app_start_inline: context state is %d, can`t start!", context->m_state);
        return -1;
    }
    context->m_state = gd_app_runing;
    return 0;
}

int gd_app_run(gd_app_context_t context) {
    int r;

    if (context->m_state != gd_app_init) {
        APP_CTX_ERROR(context, "gd_app_run: context state is %d, can`t run!", context->m_state);
        return -1;
    }

    if (gd_app_ins() != NULL) {
        CPE_ERROR(context->m_em, "gd_app_run: app already runing!");
        return -1;
    }

    context->m_state = gd_app_runing;

    gd_app_ins_set(context);
    r = gd_app_run_i(context);
    gd_app_ins_set(NULL);

    context->m_state = gd_app_shutingdown;

    gd_app_child_context_cancel_all(context);

    while(!TAILQ_EMPTY(&context->m_inline_childs)) {
        gd_app_child_context_free(TAILQ_FIRST(&context->m_inline_childs));
    }

    gd_app_child_context_wait_all(context);

    context->m_state = gd_app_done;

    return 0;
}

int gd_app_stop(gd_app_context_t context) {
    switch (context->m_state) {
    case gd_app_init:
        context->m_state = gd_app_done;
        return 0;
    case gd_app_runing: {
        context->m_state = gd_app_shutingdown;
        gd_app_child_context_cancel_all(context);
        if (context->m_stop) return context->m_stop(context, context->m_fun_ctx);
        return 0;
    }
    case gd_app_shutingdown:
        return 0;
    case gd_app_done:
        return 0;
    default:
        APP_CTX_ERROR(context, "gd_app_stop: unknown state %d!", context->m_state);
        return -1;
    }
}

int gd_app_notify_stop(gd_app_context_t context) {
    context->m_notify_stop = 1;
}

void gd_app_tick(gd_app_context_t context) {
    struct gd_app_ticker * ticker;
    gd_app_child_context_t child_context;

    if (context->m_notify_stop) {
        gd_app_stop(context);
        context->m_notify_stop = 0;
    }

    TAILQ_FOREACH(ticker, &context->m_tick_chain, m_next) {
        ticker->m_tick(ticker->m_ctx, ticker->m_arg);
    }

    TAILQ_FOREACH(child_context, &context->m_inline_childs, m_next) {
        child_context->m_tick(child_context->m_child);
    }
}

