#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
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

int otm_manage_buf_init(otm_manage_t mgr, otm_memo_t memo_buf, size_t memo_capacitiy) {
    struct cpe_hash_it timer_it;
    otm_timer_t timer;
    size_t i;

    if (cpe_hash_table_count(&mgr->m_timers) > memo_capacitiy) return -1;

    bzero(memo_buf, sizeof(struct otm_memo) * memo_capacitiy);

    cpe_hash_it_init(&timer_it, &mgr->m_timers);

    i = 0;
    while((timer = (otm_timer_t)cpe_hash_it_next(&timer_it))) {
        memo_buf[i].m_id = timer->m_id;
        memo_buf[i].m_last_action_time = 0;
        ++i;
    }

    qsort(memo_buf, memo_capacitiy, sizeof(struct otm_memo), otm_memo_cmp);

    return 0;
}

void otm_manage_tick(otm_manage_t mgr, tl_time_t cur_time, void * obj_ctx, otm_memo_t memo_buf, size_t memo_capacity) {
}

int otm_manage_enable(otm_manage_t mgr, otm_timer_id_t id, tl_time_t cur_time, otm_memo_t memo, size_t memo_capacitiy) {
    struct otm_memo key;
    otm_memo_t timer_memo;

    assert(mgr);
    assert(memo);

    key.m_id = id;

    timer_memo = (otm_memo_t)bsearch(&key, memo, memo_capacitiy, sizeof(struct otm_memo), otm_memo_cmp);
    if (timer_memo == NULL) {
        return -1;
    }

    timer_memo->m_last_action_time = cur_time;
    return 0;
}

int otm_manage_perform(otm_manage_t mgr, tl_time_t cur_time, otm_timer_id_t id, void * obj_ctx, otm_memo_t memo, size_t memo_capacitiy) {
    struct otm_memo key_memo;
    struct otm_timer key_timer;
    otm_memo_t timer_memo;
    otm_timer_t timer;

    assert(mgr);
    assert(memo);

    key_memo.m_id = id;
    timer_memo = (otm_memo_t)bsearch(&key_memo, memo, memo_capacitiy, sizeof(struct otm_memo), otm_memo_cmp);
    if (timer_memo == NULL) return -1;


    key_timer.m_id = id;
    timer = (otm_timer_t)cpe_hash_table_find(&mgr->m_timers, &key_timer);
    if (timer == NULL) return -1;

    timer->m_process(timer_memo, cur_time, timer->m_process_ctx, obj_ctx);

    return 0;
}

int otm_manage_disable(otm_manage_t mgr, otm_timer_id_t id, otm_memo_t memo, size_t memo_capacitiy) {
    struct otm_memo key;
    otm_memo_t timer_memo;

    assert(mgr);
    assert(memo);

    key.m_id = id;

    timer_memo = (otm_memo_t)bsearch(&key, memo, memo_capacitiy, sizeof(struct otm_memo), otm_memo_cmp);
    if (timer_memo == NULL) return 0;

    timer_memo->m_last_action_time = 0;
    return 0;
}

int otm_memo_cmp(void const * l, void const * r) {
    otm_memo_t l_memo = (otm_memo_t)l;
    otm_memo_t r_memo = (otm_memo_t)r;

    return l_memo->m_id < r_memo->m_id
        ? -1
        : (l_memo->m_id > r_memo->m_id
           ? 1
           : 0);
        
}
