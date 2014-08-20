#ifndef UI_SPRITE_2D_MOVE_FOLLOW_H
#define UI_SPRITE_2D_MOVE_FOLLOW_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_MOVE_FOLLOW_NAME;

ui_sprite_2d_move_follow_t ui_sprite_2d_move_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_move_follow_free(ui_sprite_2d_move_follow_t move);

int ui_sprite_2d_move_follow_contain_set_decorator(ui_sprite_2d_move_follow_t show_anim, const char * decorator);
#ifdef __cplusplus
}
#endif

#endif
