#include <assert.h>
#include "cpe/otm/otm_manage.h"
#include "otm_internal_ops.h"

otm_manage_t
otm_manage_create(
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    otm_manage_t mgr;

    mgr = (otm_manage_t)mem_alloc(alloc, sizeof(struct otm_manage));
    if (mgr == NULL) return NULL;

    mgr->m_alloc = alloc;
    mgr->m_em = em;

    if (cpe_hash_table_init(
            &mgr->m_timers,
            alloc,
            (cpe_hash_fun_t) otm_timer_hash,
            (cpe_hash_cmp_t) otm_timer_cmp,
            CPE_HASH_OBJ2ENTRY(otm_timer, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "otm_manage_create: init timer hash table fail!");
        mem_free(alloc, mgr);
        return NULL;
    }

    return mgr;
}

void otm_manage_free(otm_manage_t mgr) {
    otm_timer_free_all(mgr);
    cpe_hash_table_fini(&mgr->m_timers);
    mem_free(mgr->m_alloc, mgr);
}
