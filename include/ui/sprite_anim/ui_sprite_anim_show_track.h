#ifndef UI_SPRITE_ANIM_SHOW_TRACK_H
#define UI_SPRITE_ANIM_SHOW_TRACK_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_ANIM_SHOW_TRACK_NAME;

ui_sprite_anim_show_track_t ui_sprite_anim_show_track_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_anim_show_track_free(ui_sprite_anim_show_track_t show_track);

#ifdef __cplusplus
}
#endif

#endif
