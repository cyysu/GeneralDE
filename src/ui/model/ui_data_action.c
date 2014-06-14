#include <assert.h>
#include "ui_data_action_i.h"

ui_data_action_t ui_data_action_create(ui_data_mgr_t mgr, ui_data_src_t src) {
    ui_data_action_t action;

    if (src->m_type != ui_data_src_type_action) {
        CPE_ERROR(
            mgr->m_em, "create action at %s: src not action!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    if (src->m_product) {
        CPE_ERROR(
            mgr->m_em, "create action at %s: product already loaded!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    action = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_action));
    if (action == NULL) {
        CPE_ERROR(
            mgr->m_em, "create action at %s: alloc fail!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    action->m_mgr = mgr;
    action->m_src = src;

    TAILQ_INIT(&action->m_actors);
    TAILQ_INIT(&action->m_uses);

    src->m_product = action;

    return action;
}

void ui_data_action_free(ui_data_action_t action) {
    ui_data_mgr_t mgr = action->m_mgr;

    while(!TAILQ_EMPTY(&action->m_actors)) {
        ui_data_actor_free(TAILQ_FIRST(&action->m_actors));
    }

    assert(TAILQ_EMPTY(&action->m_uses));

    assert(action->m_src->m_product == action);
    action->m_src->m_product = NULL;

    mem_free(mgr->m_alloc, action);
}
