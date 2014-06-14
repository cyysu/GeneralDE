#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/model/ui_data_src.h"
#include "ui_data_action_i.h"
#include "ui_data_src_i.h"

ui_data_actor_t ui_data_actor_create(ui_data_action_t action) {
    ui_data_mgr_t mgr = action->m_mgr;
    ui_data_actor_t actor;

    actor = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_actor));
    if (actor == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in action %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, action->m_src));
        return NULL;
    }

    actor->m_action = action;
    bzero(&actor->m_data, sizeof(actor->m_data));

    TAILQ_INIT(&actor->m_layers);

    TAILQ_INSERT_TAIL(&action->m_actors, actor, m_next_for_action);

    return actor;
}

void ui_data_actor_free(ui_data_actor_t actor) {
    ui_data_action_t action = actor->m_action;
    ui_data_mgr_t mgr = action->m_mgr;

    while(!TAILQ_EMPTY(&actor->m_layers)) {
        ui_data_actor_layer_free(TAILQ_FIRST(&actor->m_layers));
    }

    TAILQ_REMOVE(&action->m_actors, actor, m_next_for_action);

    mem_free(mgr->m_alloc, actor);
}

static ui_data_actor_t ui_data_actor_in_action_next(ui_data_actor_it_t it) {
    ui_data_actor_t * data = (ui_data_actor_t *)(it->m_data);
    ui_data_actor_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_action);

    return r;
}

void ui_data_action_actors(ui_data_actor_it_t it, ui_data_action_t action) {
    *(ui_data_actor_t *)(it->m_data) = TAILQ_FIRST(&action->m_actors);
    it->next = ui_data_actor_in_action_next;
}

UI_ACTOR * ui_data_actor_data(ui_data_actor_t actor) {
    return &actor->m_data;
}

LPDRMETA ui_data_actor_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_actor;
}
