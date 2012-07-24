#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_require.h"
#include "logic_internal_ops.h"

static void logic_require_do_cancel(logic_require_t require);

logic_require_t
logic_require_create(logic_stack_node_t stack, const char * require_name) {
    logic_require_t require;
    logic_context_t context;
    int id_try_count;
    size_t name_len = strlen(require_name) + 1;

    context = stack->m_context;

    require = mem_alloc(context->m_mgr->m_alloc, sizeof(struct logic_require) + name_len);
    if (require == NULL) return NULL;

    require->m_context = context;
    require->m_stack = stack;
    require->m_state = logic_require_state_waiting;
    require->m_name = (char *)(require + 1);

    memcpy(require->m_name, require_name, name_len);

    cpe_hash_entry_init(&require->m_hh);
    for(id_try_count = 0; id_try_count < 2000; ++id_try_count) {
        require->m_id = context->m_mgr->m_require_id++;
        if (cpe_hash_table_insert_unique(&context->m_mgr->m_requires, require) == 0) {
            break;
        }
    }

    if (id_try_count >= 2000) {
        mem_free(context->m_mgr->m_alloc, require);
        return NULL;
    }
    
    TAILQ_INIT(&require->m_datas);

    TAILQ_INSERT_TAIL(&context->m_requires, require, m_next_for_context);
    TAILQ_INSERT_TAIL(&stack->m_requires, require, m_next_for_stack);

    ++stack->m_require_waiting_count;
    ++context->m_require_waiting_count;

    return require;
}

void logic_require_free(logic_require_t require) {
    assert(require);

    if (require->m_state == logic_require_state_waiting) {
        logic_require_do_cancel(require);
    }

    while(!TAILQ_EMPTY(&require->m_datas)) {
        logic_data_free(TAILQ_FIRST(&require->m_datas));
    }

    if (require->m_stack) {
        TAILQ_REMOVE(&require->m_stack->m_requires, require, m_next_for_stack);
    }

    TAILQ_REMOVE(&require->m_context->m_requires, require, m_next_for_context);

    cpe_hash_table_remove_by_ins(&require->m_context->m_mgr->m_requires, require);

    mem_free(require->m_context->m_mgr->m_alloc, require);
}

void logic_require_free_all(logic_manage_t mgr) {
    struct cpe_hash_it require_it;
    logic_require_t require;

    cpe_hash_it_init(&require_it, &mgr->m_requires);

    require = cpe_hash_it_next(&require_it);
    while(require) {
        logic_require_t next = cpe_hash_it_next(&require_it);
        logic_require_free(require);
        require = next;
    }
}

logic_require_t
logic_require_find(logic_manage_t mgr, logic_require_id_t id) {
    struct logic_require key;

    key.m_id = id;
    return (logic_require_t)cpe_hash_table_find(&mgr->m_requires, &key);
}

logic_require_id_t logic_require_id(logic_require_t require) {
    return require->m_id;
}

const char * logic_require_name(logic_require_t require) {
    return require->m_name;
}

logic_require_state_t logic_require_state(logic_require_t require) {
    return require->m_state;
}

logic_stack_node_t logic_require_stack(logic_require_t require) {
    return require->m_stack;
}

logic_context_t logic_require_context(logic_require_t require) {
    return require->m_context;
}

void logic_require_disconnect_to_stack(logic_require_t require) {
    if (require->m_stack) {
        if (require->m_state == logic_require_state_waiting) {
            logic_require_do_cancel(require);
        }

        TAILQ_REMOVE(&require->m_stack->m_requires, require, m_next_for_stack);
        require->m_stack = NULL;
    }
}

static void logic_require_do_cancel(logic_require_t require) {
    logic_context_state_t old_state;

    if (require->m_state != logic_require_state_waiting) return;

    old_state = logic_context_state_i(require->m_context);

    --require->m_stack->m_require_waiting_count;
    --require->m_context->m_require_waiting_count;
    require->m_state = logic_require_state_canceling;

    if (require->m_state == logic_require_state_canceling) {
        require->m_state = logic_require_state_canceled;
    }

    logic_context_do_state_change(require->m_context, old_state);
}

void logic_require_cancel(logic_require_t require) {
    logic_require_do_cancel(require);
}

void logic_require_set_done(logic_require_t require) {
    logic_context_t ctx;
    logic_context_state_t old_state;

    if (require->m_state != logic_require_state_waiting) {
        require->m_state = logic_require_state_done;
        return;
    }

    ctx = require->m_context;
    old_state = logic_context_state_i(ctx);

    assert(require->m_stack);
    --require->m_stack->m_require_waiting_count;
    --ctx->m_require_waiting_count;
    require->m_state = logic_require_state_done;

    logic_context_do_state_change(ctx, old_state);
}

void logic_require_set_error(logic_require_t require) {
    logic_context_t ctx;
    logic_context_state_t old_state;

    if (require->m_state != logic_require_state_waiting) {
        require->m_state = logic_require_state_error;
        return;
    }

    ctx = require->m_context;
    old_state = logic_context_state_i(ctx);

    assert(require->m_stack);
    --require->m_stack->m_require_waiting_count;
    --ctx->m_require_waiting_count;
    require->m_state = logic_require_state_error;

    logic_context_do_state_change(ctx, old_state);
}

uint32_t logic_require_hash(const struct logic_require * require) {
    return require->m_id;
}

int logic_require_cmp(const struct logic_require * l, const struct logic_require * r) {
    return l->m_id == r->m_id;
}
