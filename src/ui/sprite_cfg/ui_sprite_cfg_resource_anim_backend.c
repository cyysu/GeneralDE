#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite_anim/ui_sprite_anim_backend.h"
#include "ui/sprite_anim/ui_sprite_anim_module.h"
#include "ui_sprite_cfg_resource_loader_i.h"

ui_sprite_world_res_t ui_sprite_cfg_load_resource_anim_backend(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_anim_backend_t anim_backend;
    ui_sprite_anim_module_t anim_module;
    const char * anim_module_name;

    anim_module_name = cfg_get_string(cfg, "module", NULL);

    anim_module = ui_sprite_anim_module_find_nc(ui_sprite_world_app(world), anim_module_name);
    if (anim_module == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: create anim_backend resource: anim module %s not exist!",
            ui_sprite_cfg_loader_name(loader), anim_module_name ? anim_module_name : "default");
        return NULL;
    }

    anim_backend = ui_sprite_anim_backend_create(anim_module, world);
    if (anim_backend == NULL) {
        CPE_ERROR(loader->m_em, "%s: create anim_backend resource: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    return ui_sprite_world_res_from_data(anim_backend);
}
