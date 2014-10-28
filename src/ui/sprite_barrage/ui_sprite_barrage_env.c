#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_barrage_env_i.h"

static void ui_sprite_barrage_env_update(ui_sprite_world_t world, void * ctx, float delta_s);

static void ui_sprite_barrage_env_clear(ui_sprite_world_res_t world_res, void * ctx) {
    ui_sprite_barrage_env_t env = ui_sprite_world_res_data(world_res);

    plugin_barrage_env_free(env->m_env);
}

ui_sprite_barrage_env_t
ui_sprite_barrage_env_create(ui_sprite_barrage_module_t module, ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res;
    ui_sprite_barrage_env_t env;

    world_res =
        ui_sprite_world_res_create(
            world, UI_SPRITE_BARRAGE_ENV_NAME, sizeof(struct ui_sprite_barrage_env));

    if (world_res == NULL) {
        CPE_ERROR(module->m_em, "create barrage env: creat res fail!");
        return NULL;
    }

    env = ui_sprite_world_res_data(world_res);

    bzero(env, sizeof(*env));

    env->m_module = module;
    env->m_env = plugin_barrage_env_create(module->m_barrage_module);
    if (env->m_env == NULL) {
        CPE_ERROR(module->m_em, "create barrage env: creat obj mgr fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    if (ui_sprite_world_add_updator(world, ui_sprite_barrage_env_update, env) != 0) {
        CPE_ERROR(module->m_em, "create barrage env: add updator fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_barrage_env_clear, NULL);

    return env;
}

void ui_sprite_barrage_env_free(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_BARRAGE_ENV_NAME);
    if (world_res) {
        ui_sprite_world_res_free(world_res);
    }
}

ui_sprite_barrage_env_t ui_sprite_barrage_env_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_BARRAGE_ENV_NAME);
    return world_res ? ui_sprite_world_res_data(world_res) : NULL;
}

plugin_barrage_env_t ui_sprite_barrage_env_env(ui_sprite_barrage_env_t env) {
    return env->m_env;
}

int ui_sprite_barrage_env_set_update_priority(ui_sprite_barrage_env_t env, int8_t priority) {
    ui_sprite_world_res_t res = ui_sprite_world_res_from_data(env);
    ui_sprite_world_t world = ui_sprite_world_res_world(res);
    return ui_sprite_world_set_updator_priority(world, env, priority);
}

static void ui_sprite_barrage_env_update(ui_sprite_world_t world, void * ctx, float delta_s) {
    ui_sprite_barrage_env_t env = ctx;

    plugin_barrage_env_update(env->m_env, delta_s);
}

int ui_sprite_barrage_env_regist(ui_sprite_barrage_module_t module) {
    if (ui_sprite_cfg_loader_add_resource_loader(
            module->m_loader, UI_SPRITE_BARRAGE_ENV_NAME, ui_sprite_barrage_env_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_ENV_NAME);
        return -1;
    }

    return 0;
}

void ui_sprite_barrage_env_unregist(ui_sprite_barrage_module_t module) {
    if (ui_sprite_cfg_loader_remove_resource_loader(module->m_loader, UI_SPRITE_BARRAGE_ENV_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_ENV_NAME);
    }
}

const char * UI_SPRITE_BARRAGE_ENV_NAME = "BarrageEnv";
