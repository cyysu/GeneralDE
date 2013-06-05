#include "set_svr_ops.h"

set_svr_svr_t set_svr_svr_create(set_svr_t svr, set_svr_svr_type_t type, uint16_t svr_id, enum set_svr_svr_category category) {
    set_svr_svr_t svr_svr;

    svr_svr = mem_alloc(svr->m_alloc, sizeof(struct set_svr_svr));
    if (svr_svr == NULL) {
        CPE_ERROR(svr->m_em, "%s: create svr: malloc fail!", set_svr_name(svr));
        return NULL;
    }

    svr_svr->m_svr = svr;
    svr_svr->m_svr_type = type;

    svr_svr->m_category = category;
    svr_svr->m_svr_type_id = type->m_svr_type_id;
    svr_svr->m_svr_id = svr_id;

    svr_svr->m_chanel = NULL;
    svr_svr->m_router = NULL;

    cpe_hash_entry_init(&svr_svr->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_svrs, svr_svr) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: svr %d.%d: insert fail, %d.%d already exist!",
            set_svr_name(svr), svr_svr->m_svr_type_id, svr_svr->m_svr_id,
            svr_svr->m_svr_type_id, svr_svr->m_svr_id);
        mem_free(svr->m_alloc, svr_svr);
        return NULL;
    }

    if (category == set_svr_svr_local) {
        ++svr->m_local_svr_count;
        TAILQ_INSERT_TAIL(&svr->m_local_svrs, svr_svr, m_next_for_local);
    }
    
    TAILQ_INSERT_TAIL(&type->m_svrs, svr_svr, m_next_for_type);

    return svr_svr;
}

void set_svr_svr_free(set_svr_svr_t svr_svr) {
    set_svr_t svr = svr_svr->m_svr;
    set_svr_svr_type_t type = svr_svr->m_svr_type;

    if (svr_svr->m_category == set_svr_svr_local) {
        --svr->m_local_svr_count;
        TAILQ_REMOVE(&svr->m_local_svrs, svr_svr, m_next_for_local);
    }
    
    TAILQ_REMOVE(&type->m_svrs, svr_svr, m_next_for_type);

    cpe_hash_table_remove_by_ins(&svr->m_svrs, svr_svr);

    mem_free(svr->m_alloc, svr_svr);
}

void set_svr_svr_free_all(set_svr_t svr) {
    struct cpe_hash_it svr_it;
    set_svr_svr_t svr_svr;

    cpe_hash_it_init(&svr_it, &svr->m_svrs);

    svr_svr = cpe_hash_it_next(&svr_it);
    while(svr) {
        set_svr_svr_t next = cpe_hash_it_next(&svr_it);
        set_svr_svr_free(svr_svr);
        svr_svr = next;
    }
}

void set_svr_svr_set_category(set_svr_svr_t svr_svr, enum set_svr_svr_category category) {
    set_svr_t svr = svr_svr->m_svr;

    if (svr_svr->m_category == set_svr_svr_local) {
        --svr->m_local_svr_count;
        TAILQ_REMOVE(&svr->m_local_svrs, svr_svr, m_next_for_local);
    }

    if (category == set_svr_svr_local) {
        ++svr->m_local_svr_count;
        TAILQ_INSERT_TAIL(&svr->m_local_svrs, svr_svr, m_next_for_local);
    }

    svr_svr->m_category = category;
}

set_svr_svr_t set_svr_svr_find(set_svr_t svr, uint16_t svr_type_id, uint16_t svr_id) {
    struct set_svr_svr key;
    key.m_svr_type_id = svr_type_id;
    key.m_svr_id = svr_id;
    return cpe_hash_table_find(&svr->m_svrs, &key);
}

uint32_t set_svr_svr_hash(set_svr_svr_t o) {
    return (((uint32_t)o->m_svr_type_id) << 16) | o->m_svr_id;
}

int set_svr_svr_eq(set_svr_svr_t l, set_svr_svr_t r) {
    return l->m_svr_type_id == r->m_svr_type_id && l->m_svr_id == r->m_svr_id;
}
