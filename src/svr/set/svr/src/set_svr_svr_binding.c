#include "svr/set/share/set_repository.h"
#include "set_svr_ops.h"

set_svr_svr_binding_t set_svr_svr_binding_create(set_svr_svr_ins_t svr_ins, set_svr_svr_type_t type) {
    set_svr_t svr = svr_ins->m_svr;
    set_svr_svr_binding_t svr_binding;

    svr_binding = mem_alloc(svr->m_alloc, sizeof(struct set_svr_svr_binding));
    if (svr_binding == NULL) {
        CPE_ERROR(svr->m_em, "%s: create svr binding: malloc fail!", set_svr_name(svr));
        return NULL;
    }

    svr_binding->m_svr_type = type;
    svr_binding->m_svr_ins = svr_ins;
    svr_binding->m_chanel = NULL;

    cpe_hash_entry_init(&svr_binding->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_svr_bindings, svr_binding) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: svr %d.%d: insert fail, %d.%d already exist!",
            set_svr_name(svr), type->m_svr_type_id, svr_ins->m_svr_id,
            type->m_svr_type_id, svr_ins->m_svr_id);
        mem_free(svr->m_alloc, svr_binding);
        return NULL;
    }

    if (svr_ins->m_category == set_svr_svr_local) {
        ++svr->m_local_svr_count;
        TAILQ_INSERT_TAIL(&svr->m_local_svrs, svr_binding, m_next_for_local);
    }

    TAILQ_INSERT_TAIL(&type->m_runing_bindings, svr_binding, m_next_for_svr_type);
    TAILQ_INSERT_TAIL(&svr_ins->m_binding_types, svr_binding, m_next_for_svr_ins);

    return svr_binding;
}

void set_svr_svr_binding_free(set_svr_svr_binding_t svr_binding) {
    set_svr_t svr = svr_binding->m_svr_ins->m_svr;

    if (svr_binding->m_chanel) {
        set_repository_chanel_detach(svr_binding->m_chanel, svr->m_em);
        svr_binding->m_chanel = NULL;
    }
    
    if (svr_binding->m_svr_ins->m_category == set_svr_svr_local) {
        --svr->m_local_svr_count;
        TAILQ_REMOVE(&svr->m_local_svrs, svr_binding, m_next_for_local);
    }

    TAILQ_REMOVE(&svr_binding->m_svr_type->m_runing_bindings, svr_binding, m_next_for_svr_type);
    TAILQ_REMOVE(&svr_binding->m_svr_ins->m_binding_types, svr_binding, m_next_for_svr_ins);

    cpe_hash_table_remove_by_ins(&svr->m_svr_bindings, svr_binding);

    mem_free(svr->m_alloc, svr_binding);
}

set_svr_svr_binding_t set_svr_svr_binding_find(set_svr_t svr, uint16_t svr_type_id, uint16_t svr_id) {
    struct set_svr_svr_type key_type;
    struct set_svr_svr_ins key_ins;
    struct set_svr_svr_binding key;

    key_type.m_svr_type_id = svr_type_id;
    key_ins.m_svr_id = svr_id;
    key.m_svr_type = &key_type;
    key.m_svr_ins = &key_ins;

    return cpe_hash_table_find(&svr->m_svr_bindings, &key);
}

uint32_t set_svr_svr_binding_hash(set_svr_svr_binding_t o) {
    return (((uint32_t)o->m_svr_type->m_svr_type_id) << 16) | o->m_svr_ins->m_svr_id;
}

int set_svr_svr_binding_eq(set_svr_svr_binding_t l, set_svr_svr_binding_t r) {
    return l->m_svr_type->m_svr_type_id == r->m_svr_type->m_svr_type_id && l->m_svr_ins->m_svr_id == r->m_svr_ins->m_svr_id;
}
