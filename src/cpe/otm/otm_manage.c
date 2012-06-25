#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/otm/otm_manage.h"
#include "cpe/otm/otm_timer.h"
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

int otm_manage_buf_init(otm_manage_t mgr, tl_time_t cur_time, otm_memo_t memo_buf, size_t memo_capacitiy) {
    struct cpe_hash_it timer_it;
    otm_timer_t timer;
    size_t i;

    if (cpe_hash_table_count(&mgr->m_timers) > memo_capacitiy) return -1;

    bzero(memo_buf, sizeof(struct otm_memo) * memo_capacitiy);

    cpe_hash_it_init(&timer_it, &mgr->m_timers);

    i = 0;
    while((timer = (otm_timer_t)cpe_hash_it_next(&timer_it))) {
        memo_buf[i].m_id = timer->m_id;

        if (otm_timer_auto_enable(timer)) {
            memo_buf[i].m_last_action_time = cur_time;
            memo_buf[i].m_next_action_time = cur_time + timer->m_span;
        }
        else {
            memo_buf[i].m_last_action_time = 0;
            memo_buf[i].m_next_action_time = 0;
        }
        ++i;
    }

    qsort(memo_buf, memo_capacitiy, sizeof(struct otm_memo), otm_memo_cmp);

    return 0;
}

error_monitor_t otm_manage_em(otm_manage_t mgr) {
    return mgr->m_em;
}

void otm_manage_tick(otm_manage_t mgr, tl_time_t cur_time, void * obj_ctx, otm_memo_t memo_buf, size_t memo_capacity) {
    size_t i;
    for(i = 0; i < memo_capacity; ++i) {
        otm_memo_t memo = memo_buf + i;

        if (memo->m_id == 0) continue;
        if (memo->m_next_action_time == 0 || memo->m_next_action_time > cur_time) break;

        otm_timer_t timer = otm_timer_find(mgr, memo->m_id);
        if (timer == NULL) continue;

        while(memo->m_next_action_time > 0 && memo->m_next_action_time <= cur_time) {
            tl_time_t cur_next_action_time = memo->m_next_action_time;
            timer->m_process(timer, memo, cur_next_action_time, obj_ctx);
            if (memo->m_next_action_time == cur_next_action_time) {
                memo->m_next_action_time += timer->m_span;
            }
        }
    }
}

int otm_manage_enable(otm_manage_t mgr, otm_timer_id_t id, tl_time_t cur_time, tl_time_t first_exec_span, otm_memo_t memo, size_t memo_capacitiy) {
    struct otm_memo key;
    otm_memo_t timer_memo;
    otm_timer_t timer;

    assert(mgr);
    assert(memo);

    key.m_id = id;

    timer_memo = (otm_memo_t)bsearch(&key, memo, memo_capacitiy, sizeof(struct otm_memo), otm_memo_cmp);
    if (timer_memo == NULL) return -1;

    timer = otm_timer_find(mgr, id);
    if (timer == NULL) return -1;

    otm_timer_enable(timer, cur_time, first_exec_span, timer_memo);

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

    timer->m_process(timer, timer_memo, cur_time, obj_ctx);

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
    timer_memo->m_next_action_time = 0;

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


struct otm_timer_next_data {
    struct cpe_hash_it timer_it;
};

static otm_timer_t otm_timer_it_next(struct otm_timer_it * it) {
    struct otm_timer_next_data * data;

    assert(sizeof(struct otm_timer_next_data) <= sizeof(it->m_data));

    data = (struct otm_timer_next_data *)it->m_data;

    return (otm_timer_t)cpe_hash_it_next(&data->timer_it);
}

void otm_manage_timers(otm_manage_t mgr, otm_timer_it_t it) {
    struct otm_timer_next_data * data;

    assert(sizeof(struct otm_timer_next_data) <= sizeof(it->m_data));

    data = (struct otm_timer_next_data *)it->m_data;

    cpe_hash_it_init(&data->timer_it, &mgr->m_timers);

    it->next = otm_timer_it_next;
}
