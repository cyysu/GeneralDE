#ifndef UI_SPRITE_ANIM_GROUP_H
#define UI_SPRITE_ANIM_GROUP_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_anim_group_t ui_sprite_anim_group_create(ui_sprite_anim_sch_t anim_sch, const char * name);
void ui_sprite_anim_group_free(ui_sprite_anim_group_t anim_group);
ui_sprite_anim_group_t ui_sprite_anim_group_find_by_name(ui_sprite_anim_sch_t anim_sch, const char * name);

uint8_t ui_sprite_anim_group_accept_scale(ui_sprite_anim_group_t group);
void ui_sprite_anim_group_set_accept_scale(ui_sprite_anim_group_t group, uint8_t accept_scale);

UI_SPRITE_2D_PAIR ui_sprite_anim_group_pos_adj(ui_sprite_anim_group_t group);
void ui_sprite_anim_group_set_pos_adj(ui_sprite_anim_group_t group, UI_SPRITE_2D_PAIR pos_adj);

uint8_t ui_sprite_anim_group_adj_accept_scale(ui_sprite_anim_group_t group);
void ui_sprite_anim_group_set_adj_accept_scale(ui_sprite_anim_group_t group, uint8_t pos_accept_scale);

uint8_t ui_sprite_anim_group_base_pos(ui_sprite_anim_group_t group);
void ui_sprite_anim_group_set_base_pos(ui_sprite_anim_group_t group, uint8_t base_pos);

#ifdef __cplusplus
}
#endif

#endif
