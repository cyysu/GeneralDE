#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui_sprite_anim_backend_i.h"
#include "ui_sprite_anim_runing_i.h"

static void ui_sprite_anim_backend_clear(ui_sprite_world_res_t world_res, void * ctx) {
    ui_sprite_anim_backend_t backend = ui_sprite_world_res_data(world_res);

    while(!TAILQ_EMPTY(&backend->m_runing_anims)) {
        ui_sprite_anim_runing_t anim_runing = TAILQ_FIRST(&backend->m_runing_anims);
        ui_sprite_anim_runing_free(anim_runing);
    }
}

ui_sprite_anim_backend_t
ui_sprite_anim_backend_create(ui_sprite_anim_module_t module, ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res;
    ui_sprite_anim_backend_t backend;

    world_res =
        ui_sprite_world_res_create(
            world, UI_SPRITE_ANIM_BACKEND_NAME, sizeof(struct ui_sprite_anim_backend));

    if (world_res == NULL) {
        CPE_ERROR(module->m_em, "create anim backend: creat res fail!");
        return NULL;
    }

    backend = ui_sprite_world_res_data(world_res);

    bzero(backend, sizeof(*backend));

    backend->m_module = module;
    TAILQ_INIT(&backend->m_runing_anims);

    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_anim_backend_clear, NULL);

    return backend;
}

void ui_sprite_anim_backend_free(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_ANIM_BACKEND_NAME);
    if (world_res) {
        ui_sprite_world_res_free(world_res);
    }
}

ui_sprite_anim_backend_t ui_sprite_anim_backend_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_ANIM_BACKEND_NAME);
    return world_res ? ui_sprite_world_res_data(world_res) : NULL;
}

void ui_sprite_anim_backend_set_op(ui_sprite_anim_backend_t backend, struct ui_sprite_anim_backend_def * def) {
    backend->m_def = *def;
}

struct ui_sprite_anim_backend_def * ui_sprite_anim_backend_op(ui_sprite_anim_backend_t backend) {
    return &backend->m_def;
}

const char * UI_SPRITE_ANIM_BACKEND_NAME = "AnimBackend";
