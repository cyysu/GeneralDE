#include <assert.h>
#include "cpe/pom_grp/pom_grp_store.h"
#include "pom_grp_internal_ops.h"

struct pom_grp_store_entry * 
pom_grp_store_entry_create(pom_grp_store_t store, pom_grp_entry_meta_t entry_meta, const char * sub_name, LPDRMETAENTRY store_entry) {
    return NULL;
}

void pom_grp_store_entry_free(pom_grp_store_t store, struct pom_grp_store_entry * store_entry) {
}

uint32_t pom_grp_store_entry_hash(const struct pom_grp_store_entry * store_entry) {
    return store_entry->m_sub_name[0]
        ? cpe_hash_str(store_entry->m_sub_name, strlen(store_entry->m_sub_name)) 
        : cpe_hash_str(store_entry->m_entry_meta->m_name, strlen(store_entry->m_entry_meta->m_name));
}

int pom_grp_store_entry_cmp(const struct pom_grp_store_entry * l, const struct pom_grp_store_entry * r) {
    return l->m_entry_meta == r->m_entry_meta
        ? strcmp(l->m_sub_name, r->m_sub_name) == 0
        : 0;
}

void pom_grp_store_entry_free_all(pom_grp_store_t store) {
    struct cpe_hash_it stroe_entry_it;
    struct pom_grp_store_entry * stroe_entry;

    cpe_hash_it_init(&stroe_entry_it, &store->m_entries);

    stroe_entry = cpe_hash_it_next(&stroe_entry_it);
    while(stroe_entry) {
        struct pom_grp_store_entry * next = (struct pom_grp_store_entry *)cpe_hash_it_next(&stroe_entry_it);
        pom_grp_store_entry_free(store, stroe_entry);
        stroe_entry = next;
    }
}
