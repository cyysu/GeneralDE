#ifndef UI_SPRITE_2D_MOVE_H
#define UI_SPRITE_2D_MOVE_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_MOVE_NAME;

ui_sprite_2d_move_t ui_sprite_2d_move_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_move_free(ui_sprite_2d_move_t move);

#ifdef __cplusplus
}
#endif

#endif
