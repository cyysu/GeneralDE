#include "set_svr_ops.h"

set_svr_svr_ins_t set_svr_svr_ins_create(set_svr_t svr, uint16_t svr_id, enum set_svr_svr_category category) {
    set_svr_svr_ins_t svr_ins;

    svr_ins = mem_alloc(svr->m_alloc, sizeof(struct set_svr_svr_ins));
    if (svr_ins == NULL) {
        CPE_ERROR(svr->m_em, "%s: create svr: malloc fail!", set_svr_name(svr));
        return NULL;
    }

    svr_ins->m_svr = svr;

    svr_ins->m_category = category;
    svr_ins->m_svr_id = svr_id;

    svr_ins->m_router = NULL;

    TAILQ_INIT(&svr_ins->m_binding_types);

    TAILQ_INSERT_TAIL(&svr->m_svr_instances, svr_ins, m_next_for_svr);

    return svr_ins;
}

void set_svr_svr_ins_free(set_svr_svr_ins_t svr_ins) {
    set_svr_t svr = svr_ins->m_svr;

    while(!TAILQ_EMPTY(&svr_ins->m_binding_types)) {
        set_svr_svr_binding_free(TAILQ_FIRST(&svr_ins->m_binding_types));
    }

    TAILQ_REMOVE(&svr->m_svr_instances, svr_ins, m_next_for_svr);

    mem_free(svr->m_alloc, svr_ins);
}

void set_svr_svr_ins_free_all(set_svr_t svr) {
    while(TAILQ_EMPTY(&svr->m_svr_instances)) {
        set_svr_svr_ins_free(TAILQ_FIRST(&svr->m_svr_instances));
    }
}

void set_svr_svr_ins_set_category(set_svr_svr_ins_t svr_ins, enum set_svr_svr_category category) {
    set_svr_t svr = svr_ins->m_svr;

    if (svr_ins->m_category == category) return;

    if (svr_ins->m_category == set_svr_svr_local) {
        set_svr_svr_binding_t svr_binding;
        TAILQ_FOREACH(svr_binding, &svr_ins->m_binding_types, m_next_for_svr_ins) {
            --svr->m_local_svr_count;
            TAILQ_REMOVE(&svr->m_local_svrs, svr_binding, m_next_for_local);
        }
    }

    if (category == set_svr_svr_local) {
        set_svr_svr_binding_t svr_binding;
        TAILQ_FOREACH(svr_binding, &svr_ins->m_binding_types, m_next_for_svr_ins) {
            ++svr->m_local_svr_count;
            TAILQ_INSERT_TAIL(&svr->m_local_svrs, svr_binding, m_next_for_local);
        }
    }

    svr_ins->m_category = category;
}
