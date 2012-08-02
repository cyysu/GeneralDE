#include <assert.h>
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "pom_grp_internal_ops.h"

pom_grp_store_t
pom_grp_store_create(
    mem_allocrator_t alloc,
    pom_grp_meta_t meta,
    const char * main_entry,
    const char * key,
    error_monitor_t em)
{
    pom_grp_store_t store;

    store = mem_alloc(alloc, sizeof(struct pom_grp_store));
    if (store == NULL) {
        CPE_ERROR(em, "pom_grp_store_create: alloc pom_grp_store fail!");
        return NULL;
    }

    store->m_alloc = alloc;
    store->m_em = em;
    store->m_meta = meta;
    store->m_store_metalib = NULL;
    mem_buffer_init(&store->m_store_metalib_buffer, alloc);

    if (cpe_hash_table_init(
            &store->m_tables,
            alloc,
            (cpe_hash_fun_t) pom_grp_store_table_hash,
            (cpe_hash_cmp_t) pom_grp_store_table_cmp,
            CPE_HASH_OBJ2ENTRY(pom_grp_store_table, m_hh),
            -1) != 0)
    {
        mem_buffer_clear(&store->m_store_metalib_buffer);
        mem_free(alloc, store);
        return NULL;
    }

    if (pom_grp_meta_build_store_meta(&store->m_store_metalib_buffer, meta, main_entry, key, em) != 0) {
        cpe_hash_table_fini(&store->m_tables);
        mem_buffer_clear(&store->m_store_metalib_buffer);
        mem_free(alloc, store);
        return NULL;
    }

    store->m_store_metalib = (LPDRMETALIB)mem_buffer_make_continuous(&store->m_store_metalib_buffer, 0);
    assert(store->m_store_metalib);

    return store;
}

void pom_grp_store_free(pom_grp_store_t store) {
    pom_grp_store_table_free_all(store);
    cpe_hash_table_fini(&store->m_tables);
    mem_buffer_clear(&store->m_store_metalib_buffer);
    mem_free(store->m_alloc, store);
}

pom_grp_meta_t pom_grp_store_meta(pom_grp_store_t store) {
    return store->m_meta;
}

LPDRMETALIB pom_grp_store_metalib(pom_grp_store_t store) {
    return store->m_store_metalib;
}
