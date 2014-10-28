#ifndef UI_SPRITE_SPINE_PLAY_ANIM_H
#define UI_SPRITE_SPINE_PLAY_ANIM_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_PLAY_ANIM_NAME;

ui_sprite_spine_play_anim_t ui_sprite_spine_play_anim_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_play_anim_free(ui_sprite_spine_play_anim_t send_evt);

const char * ui_sprite_spine_play_anim_name(ui_sprite_spine_play_anim_t play_anim);
int ui_sprite_spine_play_anim_set_name(ui_sprite_spine_play_anim_t play_anim, const char * name);

void ui_sprite_spine_play_anim_set_is_loop(ui_sprite_spine_play_anim_t play_anim, uint8_t is_loop);
uint8_t ui_sprite_spine_play_anim_is_loop(ui_sprite_spine_play_anim_t play_anim);

#ifdef __cplusplus
}
#endif

#endif
