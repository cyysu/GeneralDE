#ifndef CPE_XCALC_PREDICATE_I_H
#define CPE_XCALC_PREDICATE_I_H
#include "cpe/xcalc/xcalc_predicate.h"
#include "xcalc_token_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xpredicate_value * xpredicate_value_t;

enum xpredicate_type {
    xpredicate_type_eq = 1
    , xpredicate_type_ne
    , xpredicate_type_bg
    , xpredicate_type_be
    , xpredicate_type_lt
    , xpredicate_type_le
    , xpredicate_type_t
    , xpredicate_type_r
    , xpredicate_type_and
    , xpredicate_type_or
    , xpredicate_type_not
};

struct xpredicate {
    enum xpredicate_type m_type;
    union {
        struct {
            xpredicate_value_t m_l;
            xpredicate_value_t m_r;
        } m_cmp;
        struct  {
            xpredicate_t m_l;
            xpredicate_t m_r;
        } m_binary_op;
        struct  {
            xpredicate_t m_i;
        } m_single_op;
    } m_data;
};

struct xpredicate_value {
    uint32_t m_type;
};

#ifdef __cplusplus
}
#endif

#endif

