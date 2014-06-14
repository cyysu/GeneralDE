#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "ui_sprite_touch_trace_i.h"
#include "ui_sprite_touch_responser_i.h"

ui_sprite_touch_trace_t ui_sprite_touch_trace_create(ui_sprite_touch_mgr_t mgr, int32_t id) {
    ui_sprite_touch_trace_t trace;

    trace = mem_alloc(mgr->m_alloc, sizeof(struct ui_sprite_touch_trace));
    if (trace == NULL) {
        CPE_ERROR(mgr->m_em, "alloc touch trace fail!");
        return NULL;
    }

    trace->m_id = id;
    trace->m_state = ui_sprite_touch_trace_new;
    TAILQ_INIT(&trace->m_active_responsers);

    TAILQ_INSERT_TAIL(&mgr->m_touch_traces, trace, m_next_for_mgr);

    return trace;
}

void ui_sprite_touch_trace_free(ui_sprite_touch_mgr_t mgr, ui_sprite_touch_trace_t trace) {
    TAILQ_REMOVE(&mgr->m_touch_traces, trace, m_next_for_mgr);

    while(!TAILQ_EMPTY(&trace->m_active_responsers)) {
        ui_sprite_touch_responser_unbind_tracer(TAILQ_FIRST(&trace->m_active_responsers), trace);
    }

    mem_free(mgr->m_alloc, trace);
}

void ui_sprite_touch_trace_free_all(ui_sprite_touch_mgr_t mgr) {
    while(!TAILQ_EMPTY(&mgr->m_touch_traces)) {
        ui_sprite_touch_trace_free(mgr, TAILQ_FIRST(&mgr->m_touch_traces));
    }
}

ui_sprite_touch_trace_t ui_sprite_touch_trace_find(ui_sprite_touch_mgr_t mgr, int32_t id) {
    ui_sprite_touch_trace_t trace;

    TAILQ_FOREACH(trace, &mgr->m_touch_traces, m_next_for_mgr) {
        if (trace->m_id == id) return trace;
    }

    return NULL;
}
