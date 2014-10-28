#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_barrage_env_i.h"

ui_sprite_world_res_t ui_sprite_barrage_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_env_t env = ui_sprite_barrage_env_create(module, world);

    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create barrage_env resource: create barrage_env fail!",
            ui_sprite_barrage_module_name(module));
        return NULL;
    }

    if (ui_sprite_barrage_env_set_update_priority(env, cfg_get_int8(cfg, "update-priority", 0)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create barrage_env resource: set update priority %d fail!",
            ui_sprite_barrage_module_name(module), cfg_get_int8(cfg, "update-priority", 0));
        ui_sprite_barrage_env_free(world);
        return NULL;
    }

    return ui_sprite_world_res_from_data(env);
}
