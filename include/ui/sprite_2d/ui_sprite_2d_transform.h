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

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_scale_pair(ui_sprite_2d_transform_t transform);
void ui_sprite_2d_transform_set_scale_pair(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR scale);

float ui_sprite_2d_transform_scale(ui_sprite_2d_transform_t transform);
void ui_sprite_2d_transform_set_scale(ui_sprite_2d_transform_t transform, float scale);

UI_SPRITE_2D_PAIR
ui_sprite_2d_transform_local_pos(
    ui_sprite_2d_transform_t transform, uint8_t pos_policy,
    uint8_t adj_type);

UI_SPRITE_2D_PAIR
ui_sprite_2d_transform_world_pos(
    ui_sprite_2d_transform_t transform, uint8_t pos_policy,
    uint8_t adj_type);

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_origin_pos(ui_sprite_2d_transform_t transform);
void ui_sprite_2d_transform_set_origin_pos(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR pos);

UI_SPRITE_2D_PAIR
ui_sprite_2d_transform_world_to_local(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR world_pt);
UI_SPRITE_2D_PAIR
ui_sprite_2d_transform_local_to_world(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR local_pt);

float ui_sprite_2d_transform_angle(ui_sprite_2d_transform_t transform);
void ui_sprite_2d_transform_set_angle(ui_sprite_2d_transform_t transform, float angle);
    
void ui_sprite_2d_transform_merge_rect(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_RECT const * rect);
UI_SPRITE_2D_RECT const * ui_sprite_2d_transform_rect(ui_sprite_2d_transform_t transform);

uint8_t ui_sprite_2d_transform_flip_x(ui_sprite_2d_transform_t transform);
uint8_t ui_sprite_2d_transform_flip_y(ui_sprite_2d_transform_t transform);
void ui_sprite_2d_transform_set_flip(ui_sprite_2d_transform_t transform, uint8_t flip_x, uint8_t flip_y);

uint8_t ui_sprite_2d_transform_pos_policy_from_str(const char * str_pos_policy);
const char * ui_sprite_2d_transform_pos_policy_to_str(uint8_t pos_policy);

float ui_sprite_2d_transform_adj_angle_by_flip(ui_sprite_2d_transform_t transform, float angle);

UI_SPRITE_2D_PAIR
ui_sprite_2d_transform_adj_local_pos(
    ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR pos, uint8_t adj_type);

UI_SPRITE_2D_PAIR
ui_sprite_2d_transform_adj_world_pos(
    ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR pos, uint8_t adj_type);

#ifdef __cplusplus
}
#endif

#endif
