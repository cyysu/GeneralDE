#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic_use/logic_require_queue.h"

struct logic_require_queue {
    char m_name[128];
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    logic_manage_t m_logic_manage;

    uint32_t m_runing_require_capacity;
    uint32_t m_runing_require_count;
    uint32_t m_runing_require_check_span;
    logic_require_id_t * m_runing_requires;

    int m_debug;
};

logic_require_queue_t
logic_require_queue_create(
    gd_app_context_t app,
    mem_allocrator_t alloc,
    error_monitor_t em,
    const char * name,
    logic_manage_t logic_manage)
{
    logic_require_queue_t queue;

    queue = mem_alloc(alloc, sizeof(struct logic_require_queue));
    if (queue == NULL) return NULL;

    strncpy(queue->m_name, name, sizeof(queue->m_name));
    queue->m_app = app;
    queue->m_alloc = alloc;
    queue->m_em = em;
    queue->m_logic_manage = logic_manage;

    queue->m_runing_require_capacity = 0;
    queue->m_runing_require_count = 0;
    queue->m_runing_require_check_span = 20000;
    queue->m_runing_requires = NULL;

    queue->m_debug = 0;

    return queue;
}

void logic_require_queue_free(logic_require_queue_t queue) {
    logic_require_queue_notify_all(queue, -1);
    if (queue->m_runing_requires) {
        mem_free(queue->m_alloc, queue->m_runing_requires);
        queue->m_runing_requires = NULL;
    }
}

int logic_require_queue_require_count(logic_require_queue_t queue) {
    return queue->m_runing_require_count;
}

logic_manage_t logic_require_queue_get_mgr(logic_require_queue_t queue) {
    return queue->m_logic_manage;
}

/* static void logic_require_queue_check_requires(logic_require_queue_t queue) { */
/*     uint32_t i; */
/*     int remove_count = 0; */

/*     for(i = 0; i < queue->m_runing_require_count; ) { */
/*         logic_require_t require = logic_require_find(queue->m_logic_manage, queue->m_runing_requires[i]); */
/*         if (require == NULL) { */
/*             if (i + 1 < queue->m_runing_require_count) { */
/*                 memmove( */
/*                     queue->m_runing_requires + i, */
/*                     queue->m_runing_requires + i + 1, */
/*                     sizeof(logic_require_id_t) * (queue->m_runing_require_count - i - 1)); */
/*             } */
/*             --queue->m_runing_require_count; */
/*             ++remove_count; */
/*         } */
/*         else { */
/*             ++i; */
/*         } */
/*     } */

/*     if (queue->m_debug) { */
/*         CPE_INFO( */
/*             queue->m_em, "%s: notify_check_requires: remove %d, remain count %d!", */
/*             queue->m_name, remove_count, queue->m_runing_require_count); */
/*     } */
/* } */

int logic_require_queue_add(logic_require_queue_t queue, logic_require_id_t id) {
    int i;

    if (queue->m_runing_require_count >= queue->m_runing_require_capacity) {
        uint32_t new_capacity;
        logic_require_id_t * new_buf;
        new_capacity = 
            queue->m_runing_require_capacity < 128 ? 128 : queue->m_runing_require_capacity * 2;
        new_buf = mem_alloc(queue->m_alloc, sizeof(logic_require_id_t) * new_capacity);
        if (new_buf == NULL) return -1;

        if (queue->m_runing_requires) {
            memcpy(new_buf, queue->m_runing_requires, sizeof(logic_require_id_t) * queue->m_runing_require_count);
            mem_free(queue->m_alloc, queue->m_runing_requires);
        }

        queue->m_runing_requires = new_buf;
        queue->m_runing_require_capacity = new_capacity;
    }

    assert(queue->m_runing_requires);
    assert(queue->m_runing_require_count < queue->m_runing_require_capacity);

    queue->m_runing_requires[queue->m_runing_require_count] = id;
    ++queue->m_runing_require_count;

    for(i = queue->m_runing_require_count - 1; i > 0; --i) {
        logic_require_id_t buf;
        if (queue->m_runing_requires[i] >= queue->m_runing_requires[i - 1]) break;

        buf = queue->m_runing_requires[i];
        queue->m_runing_requires[i] = queue->m_runing_requires[i - 1];
        queue->m_runing_requires[i - 1] = buf;
    }

    if (queue->m_debug >= 2) {
        CPE_INFO(
            queue->m_em, "%s: logic_require_queue_remove_require_id: add require %d at %d, count=%d, op-count=%d!",
            queue->m_name, id, i, queue->m_runing_require_count, queue->m_runing_require_count);
    }

    return 0;
}

int logic_require_queue_require_id_cmp(const void * l, const void * r) {
    logic_require_id_t l_id = *((const logic_require_id_t *)l);
    logic_require_id_t r_id = *((const logic_require_id_t *)r);

    return l_id < r_id ? -1
        : l_id == r_id ? 0
        : 1;
}

int logic_require_queue_remove(logic_require_queue_t queue, logic_require_id_t id) {
    logic_require_id_t * found;
    int found_pos;

    if (queue->m_runing_require_count == 0) {
        if (queue->m_debug >= 2) {
            CPE_INFO(
                queue->m_em, "%s: logic_require_queue_remove_require_id: remove require %d fail, no any require!",
                queue->m_name, id);
        }
        return -1;
    }

    assert(queue->m_runing_requires);

    found =
        (logic_require_id_t *)bsearch(
            &id,
            queue->m_runing_requires,
            queue->m_runing_require_count,
            sizeof(id),
            logic_require_queue_require_id_cmp);
    if (!found) {
        if (queue->m_debug >= 2) {
            CPE_INFO(
                queue->m_em, "%s: logic_require_queue_remove_require_id: remove require %d fail, not found!",
                queue->m_name, id);
        }
        return -1;
    }

    found_pos = found - queue->m_runing_requires;
    assert(found_pos >= 0 && (uint32_t)found_pos < queue->m_runing_require_count);

    if ((uint32_t)(found_pos + 1) < queue->m_runing_require_count) {
        memmove(found, found + 1, sizeof(logic_require_id_t) * (queue->m_runing_require_count - found_pos - 1));
    }

    --queue->m_runing_require_count;

    if (queue->m_debug >= 2) {
        CPE_INFO(
            queue->m_em, "%s: logic_require_queue_remove_require_id: remove require %d at %d, left-count=%d!",
            queue->m_name, id, found_pos, queue->m_runing_require_count);
    }

    return 0;
}

logic_require_t logic_require_queue_remove_get(logic_require_queue_t queue, logic_require_id_t id) {
    if (logic_require_queue_remove(queue, id) == 0) {
        return logic_require_find(queue->m_logic_manage, id);
    }
    else {
        return NULL;
    }
}

void logic_require_queue_notify_all(logic_require_queue_t queue, int32_t error) {
    uint32_t i;
    int notified_count = 0;

    if (error == 0) {
        for(i = 0; i < queue->m_runing_require_count; ++i) {
            logic_require_t require = logic_require_find(queue->m_logic_manage, queue->m_runing_requires[i]);
            if (require) {
                ++notified_count;
                logic_require_set_done(require);
            }
        }
    }
    else {
        for(i = 0; i < queue->m_runing_require_count; ++i) {
            logic_require_t require = logic_require_find(queue->m_logic_manage, queue->m_runing_requires[i]);
            if (require) {
                ++notified_count;
                logic_require_set_error_ex(require, error);
            }
        }
    }

    queue->m_runing_require_count = 0;

    if (queue->m_debug) {
        CPE_INFO(
            queue->m_em, "%s: notify_all: result=%d, processed %d requires!",
            queue->m_name, error, notified_count);
    }
}

void logic_require_queue_cancel_all(logic_require_queue_t queue) {
    uint32_t i;
    int notified_count = 0;

    for(i = 0; i < queue->m_runing_require_count; ++i) {
        logic_require_t require = logic_require_find(queue->m_logic_manage, queue->m_runing_requires[i]);
        if (require) {
            ++notified_count;
            logic_require_cancel(require);
        }
    }

    queue->m_runing_require_count = 0;

    if (queue->m_debug) {
        CPE_INFO(
            queue->m_em, "%s: cancel_all: processed %d requires!",
            queue->m_name, notified_count);
    }
}

