#ifndef UI_SPRITE_2D_SCALE_H
#define UI_SPRITE_2D_SCALE_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_SCALE_NAME;

ui_sprite_2d_scale_t ui_sprite_2d_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_scale_free(ui_sprite_2d_scale_t scale);

#ifdef __cplusplus
}
#endif

#endif
