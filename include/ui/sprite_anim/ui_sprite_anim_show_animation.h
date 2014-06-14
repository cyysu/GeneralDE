#ifndef UI_SPRITE_ANIM_ACTION_SHOW_ANIMATION_H
#define UI_SPRITE_ANIM_ACTION_SHOW_ANIMATION_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_ANIM_SHOW_ANIMATION_TYPE_NAME;

ui_sprite_anim_show_animation_t ui_sprite_anim_show_animation_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_anim_show_animation_free(ui_sprite_anim_show_animation_t show_animation);

const char * ui_sprite_anim_show_animation_res(ui_sprite_anim_show_animation_t show_animation);
void ui_sprite_anim_show_animation_set_res(ui_sprite_anim_show_animation_t show_animation, const char * res);

const char * ui_sprite_anim_show_animation_group(ui_sprite_anim_show_animation_t show_animation);
void ui_sprite_anim_show_animation_set_group(ui_sprite_anim_show_animation_t show_animation, const char * group);

uint8_t ui_sprite_anim_show_animation_loop(ui_sprite_anim_show_animation_t show_animation);
void ui_sprite_anim_show_animation_set_loop(ui_sprite_anim_show_animation_t show_animation, uint8_t loop);

int32_t ui_sprite_anim_show_animation_start(ui_sprite_anim_show_animation_t show_animation);
void ui_sprite_anim_show_animation_set_start(ui_sprite_anim_show_animation_t show_animation, int32_t start);

int32_t ui_sprite_anim_show_animation_end(ui_sprite_anim_show_animation_t show_animation);
void ui_sprite_anim_show_animation_set_end(ui_sprite_anim_show_animation_t show_animation, int32_t end);

#ifdef __cplusplus
}
#endif

#endif
