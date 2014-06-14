#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_cfg_loader_i.h"

int ui_sprite_cfg_load_component_transform(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_2d_transform_t transform = ui_sprite_component_data(component);
    UI_SPRITE_2D_PAIR pos;
    UI_SPRITE_2D_PAIR scale;
    float angle;

    pos.x = cfg_get_float(cfg, "pos.x", 0.0f);
    pos.y = cfg_get_float(cfg, "pos.y", 0.0f);

    scale.x = cfg_get_float(cfg, "scale.x", 1.0f);
    scale.y = cfg_get_float(cfg, "scale.y", 1.0f);

    angle = cfg_get_float(cfg, "angle", 0.0f);

    ui_sprite_2d_transform_set_pos(transform, pos);
    ui_sprite_2d_transform_set_scale(transform, scale);
    ui_sprite_2d_transform_set_angle(transform, angle);

    return 0;
}

