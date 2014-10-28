#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui_sprite_spine_env_i.h"

static void ui_sprite_spine_env_clear(ui_sprite_world_res_t world_res, void * ctx) {
    ui_sprite_spine_env_t env = ui_sprite_world_res_data(world_res);

    ui_spine_obj_mgr_free(env->m_obj_mgr);
}

ui_sprite_spine_env_t
ui_sprite_spine_env_create(ui_sprite_spine_module_t module, ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res;
    ui_sprite_spine_env_t env;

    world_res =
        ui_sprite_world_res_create(
            world, UI_SPRITE_SPINE_ENV_NAME, sizeof(struct ui_sprite_spine_env));

    if (world_res == NULL) {
        CPE_ERROR(module->m_em, "create spine env: creat res fail!");
        return NULL;
    }

    env = ui_sprite_world_res_data(world_res);

    bzero(env, sizeof(*env));

    env->m_module = module;
    env->m_obj_mgr = ui_spine_obj_mgr_create(module->m_alloc, module->m_em);
    if (env->m_obj_mgr == NULL) {
        CPE_ERROR(module->m_em, "create spine env: creat obj mgr fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_spine_env_clear, NULL);

    return env;
}

void ui_sprite_spine_env_free(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_SPINE_ENV_NAME);
    if (world_res) {
        ui_sprite_world_res_free(world_res);
    }
}

ui_sprite_spine_env_t ui_sprite_spine_env_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_SPINE_ENV_NAME);
    return world_res ? ui_sprite_world_res_data(world_res) : NULL;
}

ui_spine_obj_mgr_t ui_sprite_spine_env_obj_mgr(ui_sprite_spine_env_t env) {
    return env->m_obj_mgr;
}

const char * UI_SPRITE_SPINE_ENV_NAME = "SpineEnv";
