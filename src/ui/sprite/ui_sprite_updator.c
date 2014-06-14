#include <assert.h>
#include "ui_sprite_updator_i.h"
#include "ui_sprite_world_i.h"

ui_sprite_world_updator_t
ui_sprite_world_update_create(ui_sprite_world_t world, ui_sprite_world_update_fun_t fun, void * ctx) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_world_updator_t updator;

    updator = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_world_updator));
    if (updator == NULL) {
        CPE_ERROR(repo->m_em, "ui_sprite_world_update_crate: alloc fail!");
        return NULL;
    }

    updator->m_fun = fun;
    updator->m_ctx = ctx;

    TAILQ_INSERT_TAIL(&world->m_updators, updator, m_next);

    return updator;
}

void ui_sprite_world_update_free(ui_sprite_world_t world, ui_sprite_world_updator_t updator) {
    ui_sprite_repository_t repo = world->m_repo;

    TAILQ_REMOVE(&world->m_updators, updator, m_next);

    mem_free(repo->m_alloc, updator);
}

void ui_sprite_world_update_free_all(ui_sprite_world_t world) {
    while(!TAILQ_EMPTY(&world->m_updators)) {
        ui_sprite_world_update_free(world, TAILQ_FIRST(&world->m_updators));
    }
}
