#ifndef UI_SPRITE_2D_TRANSFORM_H
#define UI_SPRITE_2D_TRANSFORM_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_TRANSFORM_NAME;

ui_sprite_2d_transform_t ui_sprite_2d_transform_create(ui_sprite_entity_t entity);
void ui_sprite_2d_transform_free(ui_sprite_entity_t entity);
ui_sprite_2d_transform_t ui_sprite_2d_transform_find(ui_sprite_entity_t entity);

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_scale(ui_sprite_2d_transform_t transform);
void ui_sprite_2d_transform_set_scale(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR scale);

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_pos(ui_sprite_2d_transform_t transform, uint8_t pos_policy);
void ui_sprite_2d_transform_set_pos(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR pos);

float ui_sprite_2d_transform_angle(ui_sprite_2d_transform_t transform);
void ui_sprite_2d_transform_set_angle(ui_sprite_2d_transform_t transform, float angle);
    
void ui_sprite_2d_transform_merge_rect(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR lt, UI_SPRITE_2D_PAIR rb);
UI_SPRITE_2D_PAIR ui_sprite_2d_transform_rect_lt(ui_sprite_2d_transform_t transform);
UI_SPRITE_2D_PAIR ui_sprite_2d_transform_rect_rb(ui_sprite_2d_transform_t transform);

uint8_t ui_sprite_2d_transform_flip_x(ui_sprite_2d_transform_t transform);
void ui_sprite_2d_transform_set_flip_x(ui_sprite_2d_transform_t transform, uint8_t flip_x);

uint8_t ui_sprite_2d_transform_flip_y(ui_sprite_2d_transform_t transform);
void ui_sprite_2d_transform_set_flip_y(ui_sprite_2d_transform_t transform, uint8_t flip_y);

uint8_t ui_sprite_2d_transform_pos_policy_from_str(const char * str_pos_policy);
const char * ui_sprite_2d_transform_pos_policy_to_str(uint8_t pos_policy);

#ifdef __cplusplus
}
#endif

#endif
