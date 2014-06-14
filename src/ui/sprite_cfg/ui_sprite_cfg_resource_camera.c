#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite_anim/ui_sprite_anim_camera.h"
#include "ui/sprite_anim/ui_sprite_anim_module.h"
#include "ui_sprite_cfg_resource_loader_i.h"

ui_sprite_world_res_t ui_sprite_cfg_load_resource_camera(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_anim_camera_t camera;
    ui_sprite_anim_module_t anim_module;
    const char * anim_module_name;
    UI_SPRITE_2D_PAIR limit_lt;
    UI_SPRITE_2D_PAIR limit_rb;
    cfg_t limit_lt_cfg;
    cfg_t limit_rb_cfg;

    anim_module_name = cfg_get_string(cfg, "module", NULL);

    anim_module = ui_sprite_anim_module_find_nc(ui_sprite_world_app(world), anim_module_name);
    if (anim_module == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: create camera resource: anim module %s not exist!",
            ui_sprite_cfg_loader_name(loader), anim_module_name ? anim_module_name : "default");
        return NULL;
    }

    camera = ui_sprite_anim_camera_create(anim_module, world);
    if (camera == NULL) {
        CPE_ERROR(loader->m_em, "%s: create camera resource: limit  (%f,%f) - (%f,%f) error!",
            ui_sprite_cfg_loader_name(loader), limit_lt.x, limit_lt.y, limit_rb.x, limit_rb.y);
        ui_sprite_anim_camera_free(camera);
        return NULL;
    }

    limit_lt = ui_sprite_anim_camera_limit_lt(camera);
    limit_rb = ui_sprite_anim_camera_limit_rb(camera);

    if ((limit_lt_cfg = cfg_find_cfg(cfg, "limit.lt"))) {
        limit_lt.x = cfg_get_float(limit_lt_cfg, "x", limit_lt.x);
        limit_lt.y = cfg_get_float(limit_lt_cfg, "y", limit_lt.y);
    }

    if ((limit_rb_cfg = cfg_find_cfg(cfg, "limit.br"))) {
        limit_rb.x = cfg_get_float(limit_rb_cfg, "x", limit_rb.x);
        limit_rb.y = cfg_get_float(limit_rb_cfg, "y", limit_rb.y);
    }

    ui_sprite_anim_camera_set_limit(camera, limit_lt, limit_rb);

    return ui_sprite_world_res_from_data(camera);
}
