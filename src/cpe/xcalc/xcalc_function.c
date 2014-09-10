#include "cpe/pal/pal_string.h"
#include "xcalc_computer_i.h"

xtoken_t xcomputer_func_strlen(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em) {
    xtoken_t arg = xtoken_it_next(args);
    xtoken_t r;

    if (arg == NULL) {
        CPE_ERROR(em, "strlen: no arg!");
        return NULL;
    }

    if (xtoken_it_next(args) != NULL) {
        CPE_ERROR(em, "strlen: too many arg!");
        return NULL;
    }

    if (arg->m_type != XTOKEN_STRING) {
        CPE_ERROR(em, "strlen: arg is not string!");
        return NULL;
    }

    r = xcomputer_alloc_token(computer);
    if (r == NULL) {
        CPE_ERROR(em, "strlen: alloc result fail!");
        return NULL;
    }

    xcomputer_set_token_int(computer, r, strlen(arg->m_data.str._string));

    return r;
}

static struct {
    const char * m_func_name;
    xcalc_func_t m_fun;
} g_default_funs[] = {
    { "strlen", xcomputer_func_strlen }
};

int xcomputer_load_default_funcs(xcomputer_t computer) {
    int i;

    for(i = 0; i < CPE_ARRAY_SIZE(g_default_funs); ++i) {
        if (xcomputer_add_func(computer, g_default_funs[i].m_func_name, g_default_funs[i].m_fun, NULL) != 0) {
            CPE_ERROR(computer->m_em, "xcomputer: load default funcs: add func %s fail!", g_default_funs[i].m_func_name);
            return -1;
        }
    }

    return 0;
}
