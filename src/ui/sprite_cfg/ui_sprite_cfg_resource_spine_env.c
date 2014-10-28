#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite_spine/ui_sprite_spine_env.h"
#include "ui/sprite_spine/ui_sprite_spine_module.h"
#include "ui_sprite_cfg_resource_loader_i.h"

ui_sprite_world_res_t ui_sprite_cfg_load_resource_spine_env(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_spine_module_t spine_module;
    ui_sprite_spine_env_t spine_env;
    const char * spine_module_name;

    spine_module_name = cfg_get_string(cfg, "module", NULL);

    spine_module = ui_sprite_spine_module_find_nc(ui_sprite_world_app(world), spine_module_name);
    if (spine_module == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: create spine_env resource: spine module %s not exist!",
            ui_sprite_cfg_loader_name(loader), spine_module_name ? spine_module_name : "default");
        return NULL;
    }

    spine_env = ui_sprite_spine_env_create(spine_module, world);
    if (spine_env == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: create spine_env resource: create spine_env fail!",
            ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    return ui_sprite_world_res_from_data(spine_env);
}
