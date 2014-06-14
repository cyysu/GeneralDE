#ifndef UI_SPRITE_2D_TYPES_H
#define UI_SPRITE_2D_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_2d_module * ui_sprite_2d_module_t;
typedef struct ui_sprite_2d_transform * ui_sprite_2d_transform_t;
typedef struct ui_sprite_2d_move * ui_sprite_2d_move_t;
typedef struct ui_sprite_2d_scale * ui_sprite_2d_scale_t;
typedef struct ui_sprite_2d_track_flip * ui_sprite_2d_track_flip_t;
typedef struct ui_sprite_2d_track_angle * ui_sprite_2d_track_angle_t;
typedef struct ui_sprite_2d_wait_switchback * ui_sprite_2d_wait_switchback_t;

#ifdef __cplusplus
}
#endif

#endif


