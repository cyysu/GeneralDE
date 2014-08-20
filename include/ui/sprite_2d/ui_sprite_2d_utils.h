#ifndef UI_SPRITE_2D_UTILS_H
#define UI_SPRITE_2D_UTILS_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ui_sprite_2d_pt_in_rect(UI_SPRITE_2D_PAIR check_pt, UI_SPRITE_2D_RECT const * rect);
uint8_t ui_sprite_2d_pt_in_circle(UI_SPRITE_2D_PAIR check_pt, UI_SPRITE_2D_PAIR const * center, float radius);
uint8_t ui_sprite_2d_rect_in_rect(UI_SPRITE_2D_RECT const * check, UI_SPRITE_2D_RECT const * rect);

uint8_t ui_sprite_2d_pt_eq(UI_SPRITE_2D_PAIR p1, UI_SPRITE_2D_PAIR p2, float delta);
uint8_t ui_sprite_2d_rect_eq(UI_SPRITE_2D_RECT const * r1, UI_SPRITE_2D_RECT const * r2, float delta);

uint8_t ui_sprite_2d_rect_merge(UI_SPRITE_2D_RECT * target, UI_SPRITE_2D_RECT const * input);

int ui_sprite_2d_merge_contain_rect_group(UI_SPRITE_2D_PAIR * lt, UI_SPRITE_2D_PAIR * rb, ui_sprite_group_t group);
int ui_sprite_2d_merge_contain_rect_entity(UI_SPRITE_2D_PAIR * lt, UI_SPRITE_2D_PAIR * rb, ui_sprite_entity_t entity);

#ifdef __cplusplus
}
#endif

#endif


