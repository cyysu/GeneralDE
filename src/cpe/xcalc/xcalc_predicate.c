#include <assert.h>
#include "xcalc_predicate_i.h"
#include "xcalc_scaner_i.h"

static xpredicate_t xpredicate_alloc(mem_allocrator_t alloc, enum xpredicate_type type, error_monitor_t em);
static void xpredicate_value_free(mem_allocrator_t alloc, xpredicate_value_t value);

xpredicate_t xpredicate_parse(mem_allocrator_t alloc, const char * exp, error_monitor_t em) {
    xpredicate_t root = NULL;
    xscaner_t scaner = NULL;
    struct xtoken token;

    scaner = xscaner_create(alloc, exp);
    if (scaner == NULL) {
        CPE_ERROR(em, "xpredicate: parse %s: alloc scaner fail!", exp);
        goto PARSE_ERROR;
    }

    /* do { */
    /*     int r = xscaner_get_token(scaner, &token, em); */
    /*     if (r < 0) { */
    /*         CPE_ERROR(em, "xpredicate: parse %s: get token fail!", exp); */
    /*         goto PARSE_ERROR; */
    /*     } */

        
    /* } while(0); */
    
    if (root == NULL) {
        root = xpredicate_alloc(alloc, xpredicate_type_t, em);
    }

    if (scaner) xscaner_free(scaner);

    return root;

PARSE_ERROR:
    if (scaner) xscaner_free(scaner);
    if (root) xpredicate_free(alloc, root);
    return NULL;
}

void xpredicate_free(mem_allocrator_t alloc, xpredicate_t pred) {
    switch(pred->m_type) {
    case xpredicate_type_eq:
    case xpredicate_type_ne:
    case xpredicate_type_bg:
    case xpredicate_type_be:
    case xpredicate_type_lt:
    case xpredicate_type_le:
        xpredicate_value_free(alloc, pred->m_data.m_cmp.m_l);
        xpredicate_value_free(alloc, pred->m_data.m_cmp.m_r);
        break;
    case xpredicate_type_t:
    case xpredicate_type_r:
        break;
    case xpredicate_type_and:
    case xpredicate_type_or:
        xpredicate_free(alloc, pred->m_data.m_binary_op.m_l);
        xpredicate_free(alloc, pred->m_data.m_binary_op.m_r);
        break;
    case xpredicate_type_not:
        xpredicate_free(alloc, pred->m_data.m_single_op.m_i);
        break;
    default:
        assert(0);
    }

    mem_free(alloc, pred);
}

int xpredicate_eval(xpredicate_t pred) {
    switch(pred->m_type) {
    case xpredicate_type_eq:
    case xpredicate_type_ne:
    case xpredicate_type_bg:
    case xpredicate_type_be:
    case xpredicate_type_lt:
    case xpredicate_type_le:
        return 1;
    case xpredicate_type_t:
        return 1;
    case xpredicate_type_r:
        return 0;
    case xpredicate_type_and:
        return xpredicate_eval(pred->m_data.m_binary_op.m_l) && xpredicate_eval(pred->m_data.m_binary_op.m_r);
    case xpredicate_type_or:
        return xpredicate_eval(pred->m_data.m_binary_op.m_l) || xpredicate_eval(pred->m_data.m_binary_op.m_r);
        break;
    case xpredicate_type_not:
        return ! xpredicate_eval(pred->m_data.m_single_op.m_i);
    default:
        assert(0);
        return 0;
    }
}

static xpredicate_t xpredicate_alloc(mem_allocrator_t alloc, enum xpredicate_type type, error_monitor_t em) {
    xpredicate_t predicate = mem_alloc(alloc, sizeof(struct xpredicate));
    if (predicate == NULL) {
        CPE_ERROR(em, "alloc xpredicate fail!");
        return NULL;
    }

    predicate->m_type = type;

    return predicate;
}

static void xpredicate_value_free(mem_allocrator_t alloc, xpredicate_value_t value) {
    mem_free(alloc, value);
}
