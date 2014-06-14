#ifndef UI_SPRITE_TOUCH_BOX_H
#define UI_SPRITE_TOUCH_BOX_H
#include "ui_sprite_touch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_touch_box_t
ui_sprite_touch_box_create(ui_sprite_touch_touchable_t touchable, UI_SPRITE_2D_PAIR lt, UI_SPRITE_2D_PAIR rb);

void ui_sprite_touch_box_free(ui_sprite_touch_box_t box);

UI_SPRITE_2D_PAIR ui_sprite_touch_box_lt(ui_sprite_touch_box_t box);
UI_SPRITE_2D_PAIR ui_sprite_touch_box_rb(ui_sprite_touch_box_t box);

#ifdef __cplusplus
}
#endif

#endif
