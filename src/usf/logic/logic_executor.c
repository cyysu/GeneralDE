#include <assert.h>
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_context.h"
#include "logic_internal_ops.h"

logic_executor_t
logic_executor_action_create(logic_manage_t mgr, logic_executor_type_t type, cfg_t args) {
    struct logic_executor_action * executor;
    cfg_t executor_args;

    assert(mgr);

    if (type == NULL) return NULL;

    executor_args = NULL;
    if (args) {
        if (cfg_type(args) != CPE_CFG_TYPE_STRUCT) return NULL;
        executor_args = cfg_create(mgr->m_alloc);
        if (executor_args == NULL) return NULL;

        if (cfg_merge(executor_args, args, cfg_replace, NULL) != 0) {
            cfg_free(executor_args);
            return NULL;
        }
    }

    executor = (struct logic_executor_action *)mem_alloc(mgr->m_alloc, sizeof(struct logic_executor_action));
    if (executor == NULL) {
        if (executor_args) cfg_free(executor_args);
        return NULL;
    }

    executor->m_mgr = mgr;
    executor->m_category = logic_executor_category_action;
    executor->m_type = type;
    executor->m_args = executor_args;

    return (logic_executor_t)executor;
}

logic_executor_t
logic_executor_composite_create(logic_manage_t mgr, logic_executor_composite_type_t composite_type) {
    struct logic_executor_composite * executor;

    assert(mgr);

    executor = (struct logic_executor_composite *)mem_alloc(mgr->m_alloc, sizeof(struct logic_executor_composite));
    if (executor == NULL) return NULL;

    executor->m_mgr = mgr;
    executor->m_category = logic_executor_category_composite;
    executor->m_composite_type = composite_type;
    TAILQ_INIT(&executor->m_members);

    return (logic_executor_t)executor;
}

int logic_executor_composite_add(logic_executor_t input_composite, logic_executor_t member) {
    struct logic_executor_composite * composite;

    assert(input_composite);
    if (member == NULL) return -1;
    if (input_composite->m_category != logic_executor_category_composite) return -1;

    composite = (struct logic_executor_composite *)input_composite;

    TAILQ_INSERT_TAIL(&composite->m_members, member, m_next);

    return 0;
}

logic_executor_t
logic_executor_decorator_create(logic_manage_t mgr, logic_executor_decorator_type_t decorator_type, logic_executor_t inner) {
    struct logic_executor_decorator * executor;

    assert(mgr);
    if (inner == NULL) return NULL;

    executor = (struct logic_executor_decorator *)mem_alloc(mgr->m_alloc, sizeof(struct logic_executor_decorator));
    if (executor == NULL) return NULL;

    executor->m_mgr = mgr;
    executor->m_category = logic_executor_category_decorator;
    executor->m_decorator_type = decorator_type;
    executor->m_inner = inner;

    return (logic_executor_t)executor;
}

void logic_executor_free(logic_executor_t executor) {
    if (executor == NULL) return;

    switch(executor->m_category) {
    case logic_executor_category_action: {
        struct logic_executor_action * action = (struct logic_executor_action *)executor;
        if (action->m_args) cfg_free(action->m_args);
        mem_free(action->m_mgr->m_alloc, action);
        break;
    }
    case logic_executor_category_composite: {
        logic_executor_t member;
        struct logic_executor_composite * composite = (struct logic_executor_composite *)executor;

        while(!TAILQ_EMPTY(&composite->m_members)) {
            member = TAILQ_FIRST(&composite->m_members);
            TAILQ_REMOVE(&composite->m_members, member, m_next);
            logic_executor_free(member);
        }

        mem_free(composite->m_mgr->m_alloc, composite);
        break;
    }
    case logic_executor_category_decorator: {
        struct logic_executor_decorator * decorator = (struct logic_executor_decorator *)executor;
        logic_executor_free(decorator->m_inner);
        mem_free(decorator->m_mgr->m_alloc, decorator);
        break;
    }
    case logic_executor_category_condition: {
        struct logic_executor_condition * condition = (struct logic_executor_condition *)executor;
        mem_free(condition->m_mgr->m_alloc, condition);
    }
    }
}

const char * logic_executor_name(logic_executor_t executor) {
    switch(executor->m_category) {
    case logic_executor_category_action:
        return ((struct logic_executor_action *)executor)->m_type->m_name;
    case logic_executor_category_condition:
        return "condition";
    case logic_executor_category_decorator:
        switch (((struct logic_executor_decorator *)executor)->m_decorator_type) {
        case logic_executor_decorator_protect:
            return "protect";
        case logic_executor_decorator_not:
            return "not";
        default:
            return "unknown-decorator-type";
        }
    case logic_executor_category_composite:
        switch (((struct logic_executor_composite *)executor)->m_composite_type) {
        case logic_executor_composite_selector:
            return "selector";
        case logic_executor_composite_sequence:
            return "sequence";
        case logic_executor_composite_parallel:
            return "parallel";
        default:
            return "unknown-composite-type";
        }
    default:
        return "unknown-executor-category";
    }
}

void logic_executor_dump(logic_executor_t executor, write_stream_t stream, int level) {
    if (executor == NULL) return;

    switch(executor->m_category) {
    case logic_executor_category_action: {
        struct logic_executor_action * action = (struct logic_executor_action *)executor;
        stream_putc_count(stream, ' ', level << 2);
        if (action->m_args) {
            stream_printf(stream, "%s: ", logic_executor_name(executor));
            cfg_dump_inline(action->m_args, stream);
        }
        else {
            stream_printf(stream, "%s", logic_executor_name(executor));
        }
        break;
    }
    case logic_executor_category_composite: {
        logic_executor_t member;
        struct logic_executor_composite * composite = (struct logic_executor_composite *)executor;
        stream_putc_count(stream, ' ', level << 2);
        stream_printf(stream, "%s:", logic_executor_name(executor));

        TAILQ_FOREACH(member, &composite->m_members, m_next) {
            stream_putc(stream, '\n');
            logic_executor_dump(member, stream, level + 1);
        }

        break;
    }
    case logic_executor_category_decorator: {
        struct logic_executor_decorator * decorator = (struct logic_executor_decorator *)executor;
        stream_putc_count(stream, ' ', level << 2);
        stream_printf(stream, "%s:", logic_executor_name(executor));
        stream_putc(stream, '\n');
        logic_executor_dump(decorator->m_inner, stream, level + 1);
        break;
    }
    case logic_executor_category_condition: {
        stream_putc_count(stream, ' ', level << 2);
        stream_printf(stream, "%s", logic_executor_name(executor));
        break;
    }
    default:
        stream_putc_count(stream, ' ', level << 2);
        stream_printf(stream, "%s", logic_executor_name(executor));
        break;
    }
}
