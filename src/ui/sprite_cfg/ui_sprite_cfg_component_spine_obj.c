#include "cpe/utils/stream_mem.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_spine/ui_sprite_spine_obj.h"
#include "ui_sprite_cfg_loader_i.h"

int ui_sprite_cfg_load_component_spine_obj(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_spine_obj_t spine_obj = ui_sprite_component_data(component);
    const char * path;

    if ((path = cfg_get_string(cfg, "obj-path", NULL))) {
        ui_sprite_spine_obj_set_obj_path(spine_obj, path);
    }

    ui_sprite_spine_obj_set_debug_slots(spine_obj, cfg_get_uint8(cfg, "debug-slots", 0));
    ui_sprite_spine_obj_set_debug_bones(spine_obj, cfg_get_uint8(cfg, "debug-bones", 0));

    return 0;
}
