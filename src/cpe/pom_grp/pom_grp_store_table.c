#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "pom_grp_internal_ops.h"

pom_grp_store_table_t
pom_grp_store_table_create(pom_grp_store_t store, LPDRMETA meta) {
    pom_grp_store_table_t table;

    table = (pom_grp_store_table_t)mem_alloc(store->m_alloc, sizeof(struct pom_grp_store_table));
    if (table == NULL) return NULL;

    table->m_store = store;
    table->m_meta = meta;

    cpe_hash_entry_init(&table->m_hh);

    if (cpe_hash_table_insert_unique(&store->m_tables, table) != 0) {
        mem_free(store->m_alloc, table);
        return NULL; 
    }

    return table;
}

void pom_grp_store_table_free(pom_grp_store_table_t store_table) {
    cpe_hash_table_remove_by_ins(&store_table->m_store->m_tables, store_table);
    mem_free(store_table->m_store->m_alloc, store_table);
}

uint32_t pom_grp_store_table_hash(const struct pom_grp_store_table * store_table) {
    const char * name = dr_meta_name(store_table->m_meta);
    return cpe_hash_str(name, strlen(name));
}

int pom_grp_store_table_cmp(const struct pom_grp_store_table * l, const struct pom_grp_store_table * r) {
    return l->m_meta == r->m_meta;
}

void pom_grp_store_table_free_all(pom_grp_store_t store) {
    struct cpe_hash_it stroe_table_it;
    struct pom_grp_store_table * stroe_table;

    cpe_hash_it_init(&stroe_table_it, &store->m_tables);

    stroe_table = cpe_hash_it_next(&stroe_table_it);
    while(stroe_table) {
        struct pom_grp_store_table * next = (struct pom_grp_store_table *)cpe_hash_it_next(&stroe_table_it);
        pom_grp_store_table_free(stroe_table);
        stroe_table = next;
    }
}
