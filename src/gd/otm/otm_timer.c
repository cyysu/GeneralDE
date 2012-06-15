#include <assert.h>
#include "otm_internal_ops.h"

uint32_t otm_timer_hash(const struct otm_timer * context) {
    return (uint32_t)context->m_id;
}

int otm_timer_cmp(const struct otm_timer * l, const struct otm_timer * r) {
    return l->m_id == r->m_id;
}

