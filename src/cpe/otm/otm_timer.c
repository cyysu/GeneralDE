#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "otm_internal_ops.h"

uint32_t otm_timer_hash(const struct otm_timer * context) {
    return (uint32_t)context->m_id;
}

int otm_timer_cmp(const struct otm_timer * l, const struct otm_timer * r) {
    return l->m_id == r->m_id;
}

otm_timer_t otm_timer_create(
    otm_manage_t mgr,
    otm_timer_id_t id,
    const char * name,
    otm_process_fun_t process,
    void * process_ctx)
{
    otm_timer_t timer;
    size_t name_len = strlen(name) + 1;

    timer = (otm_timer_t)mem_alloc(mgr->m_alloc, sizeof(struct otm_timer) + name_len);
    if (timer == NULL) return NULL;

    timer->m_mgr = mgr;
    timer->m_id = id;
    timer->m_name = (const char *)(timer + 1);
    timer->m_process = process;
    timer->m_process_ctx = process_ctx;

    cpe_hash_entry_init(&timer->m_hh);
    if (cpe_hash_table_insert_unique(&mgr->m_timers, timer) != 0) {
        mem_free(mgr->m_alloc, timer);
        return NULL;
    }

    return timer;
}

void otm_timer_free(otm_timer_t timer) {
    otm_manage_t mgr;

    assert(timer);

    mgr = timer->m_mgr;

    cpe_hash_table_remove_by_ins(&mgr->m_timers, timer);

    mem_free(mgr->m_alloc, timer);
}

void otm_timer_free_all(otm_manage_t mgr) {
}

