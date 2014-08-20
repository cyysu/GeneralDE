#ifndef UI_SPRITE_TOUCH_BOX_H
#define UI_SPRITE_TOUCH_BOX_H
#include "ui_sprite_touch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_touch_box_t
ui_sprite_touch_box_create(
    ui_sprite_touch_touchable_t touchable, 
    UI_SPRITE_TOUCH_SHAPE const * shape);

void ui_sprite_touch_box_free(ui_sprite_touch_box_t box);

UI_SPRITE_TOUCH_SHAPE const * ui_sprite_touch_box_shape(ui_sprite_touch_box_t box);

uint8_t ui_sprite_touch_box_check_pt_in(
    ui_sprite_touch_box_t box, ui_sprite_entity_t entity, UI_SPRITE_2D_PAIR test_pt);

#ifdef __cplusplus
}
#endif

#endif
