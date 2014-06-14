#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_touch/ui_sprite_touch_touchable.h"
#include "ui/sprite_touch/ui_sprite_touch_box.h"
#include "ui_sprite_cfg_loader_i.h"

int ui_sprite_cfg_load_component_touchable(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_touch_touchable_t touchable = ui_sprite_component_data(component);
    ui_sprite_cfg_loader_t loader = ctx;
    struct cfg_it child_cfg_it;
    cfg_t child_cfg;

    cfg_it_init(&child_cfg_it, cfg_find_cfg(cfg, "boxes"));
    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        UI_SPRITE_2D_PAIR lt = { cfg_get_float(child_cfg, "lt.x", 0.0f), cfg_get_float(child_cfg, "lt.y", 0.0f) };
        UI_SPRITE_2D_PAIR rb = { cfg_get_float(child_cfg, "rb.x", 0.0f), cfg_get_float(child_cfg, "rb.y", 0.0f) };
        ui_sprite_touch_box_t box;

        if (rb.x <= lt.x || rb.y <= lt.y) {
            CPE_ERROR(
                loader->m_em, "%s: entity %d(%s): create Touchable: box lt=(%f,%f), rb=(%f,%f) error!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                lt.x, lt.y, rb.x, rb.y);
            return -1;
        }

        box = ui_sprite_touch_box_create(touchable, lt, rb);
        if (box == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: entity %d(%s): create Touchable: create box fail!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }

    return 0;
}

